//
// Created by hb on 11/2/25.
//

#include "FAT16.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>

FAT16::FAT16(const string& diskImagePath) :
    m_osManager(diskImagePath),
    m_isMounted(false) {
    memset(&m_bootSector, 0, sizeof(BootSectorFAT16));
}

bool FAT16::getIsMounted() {
    return m_isMounted;
}

bool FAT16::mount() {
    if (!m_osManager.isDiskOpen()) {
        cerr << "Erro: Disco fechado" << endl;
        return false;
    }

    vector<uint8_t> vbrBuffer(512);
    if (!m_osManager.readSectors(0, 1, vbrBuffer.data())) {
        cerr << "Erro: Falha ao ler o Setor de Boot" << endl;
        return false;
    }

    memcpy(&m_bootSector, vbrBuffer.data(), sizeof(BootSectorFAT16));

    if (m_bootSector.bootableSignature != 0xAA55) {
        cerr << "Erro: Assinatura de boot (0xAA55) invalida." << endl;
        return false;
    }

    if (m_bootSector.bytesPerSector != 512) {
        cerr << "Erro: Tamanho de setor nao suportado (nao e 512)." << endl;
        return false;
    }

    if (strncmp(m_bootSector.fileSystemType, "FAT16", 5) != 0) {
        cerr << "Erro: Sistema de arquivos nao e FAT16." << endl;
        return false;
    }


    m_fatStartSector = m_bootSector.reservedSectors;

    m_rootDirStartSector = m_fatStartSector + (m_bootSector.numFATs * m_bootSector.sectorsPerFAT);

    uint32_t rootDirSizeBytes = m_bootSector.numRootDirEntries * sizeof(DirectoryEntryFAT16); // 512 * 32 = 16384

    m_rootDirSectors = (rootDirSizeBytes + m_bootSector.bytesPerSector - 1) / m_bootSector.bytesPerSector; // 16384 / 512 = 32

    m_dataStartSector = m_rootDirStartSector + m_rootDirSectors;

    m_bytesPerCluster = m_bootSector.sectorsPerCluster * m_bootSector.bytesPerSector;

    uint32_t fatSizeInBytes = m_bootSector.sectorsPerFAT * m_bootSector.bytesPerSector;

    m_fatCache.resize(fatSizeInBytes / 2);

    if (!m_osManager.readSectors(m_fatStartSector, m_bootSector.sectorsPerFAT, (uint8_t*)m_fatCache.data())) {
        cerr << "Erro: Falha ao carregar a FAT para o cache." << endl;
        return false;
    }

    cout << "FAT16 montado com sucesso!" << endl;
    cout << "  FAT comeca em: " << m_fatStartSector << endl;
    cout << "  Root Dir comeca em: " << m_rootDirStartSector << " (tamanho: " << m_rootDirSectors << " setores)" << endl;
    cout << "  Dados comecam em: " << m_dataStartSector << endl;

    m_isMounted = true;
    return true;
}

long FAT16::findRootEntry(const string& filename, DirectoryEntryFAT16& outEntry) {
    vector<uint8_t> rootDirBuffer(m_rootDirSectors * m_bootSector.bytesPerSector);

    if (!m_osManager.readSectors(m_rootDirStartSector, m_rootDirSectors, rootDirBuffer.data())) {
        cerr << "Erro: Nao foi possivel ler o diretorio raiz (em findRootEntry)." << endl;
        return -1; // Falha na leitura
    }

    auto* entries = (DirectoryEntryFAT16*)rootDirBuffer.data();
    for (uint32_t i = 0; i < m_bootSector.numRootDirEntries; i++) {
        DirectoryEntryFAT16& entry = entries[i];

        if (entry.fileName[0] == 0x00) {
            break;
        }

        if ((uint8_t)entry.fileName[0] == 0xE5) {
            continue;
        }

        if (entry.attributes == 0x0F) {
            continue;
        }

        if ((entry.attributes & 0x10) == 0x10) {
            continue;
        }

        if (formatFilename(entry) == filename) {
            outEntry = entry;

            long rootDirByteOffset = m_rootDirStartSector * m_bootSector.bytesPerSector;
            return rootDirByteOffset + (i * sizeof(DirectoryEntryFAT16));
        }
    }

    return -1; // Arquivo não encontrado
}

long FAT16::getFileSize(const string& filename) {
    if (!m_isMounted) {
        cerr << "Erro: Sistema nao montado." << endl;
        return -1;
    }

    DirectoryEntryFAT16 entry;

    long entryOffset = findRootEntry(filename, entry);

    if (entryOffset < 0) {
        return -1;
    }

    return (long)entry.fileSize;
}

string FAT16::formatFilename(const DirectoryEntryFAT16& entry) {
    string name(entry.fileName, 8);
    string ext(entry.extension, 3);

    name.erase(name.find_last_not_of(' ') + 1);
    ext.erase(ext.find_last_not_of(' ') + 1);

    if (ext.empty()) {
        return name;
    }

    return name + "." + ext;
}

vector<FileInfo> FAT16::listRootDirectory() {
    vector<FileInfo> fileList; // MUDANÇA: O vetor agora é de FileInfo

    if (!m_isMounted) {
        cerr << "Erro: Sistema nao montado. Chame mount() primeiro." << endl;
        return fileList;
    }

    vector<uint8_t> rootDirBuffer(m_rootDirSectors * m_bootSector.bytesPerSector);

    if (!m_osManager.readSectors(m_rootDirStartSector, m_rootDirSectors, rootDirBuffer.data())) {
        cerr << "Erro: Nao foi possivel ler o diretorio raiz." << endl;
        return fileList;
    }

    auto* entries = (DirectoryEntryFAT16*)rootDirBuffer.data();

    for (uint32_t i = 0; i < m_bootSector.numRootDirEntries; i++) {
        DirectoryEntryFAT16& entry = entries[i];

        // ... (todas as suas verificações 'if/continue' continuam iguais)
        if (entry.fileName[0] == 0x00) break;
        if ((uint8_t)entry.fileName[0] == 0xE5) continue;
        if (entry.attributes == 0x0F) continue;
        if ((entry.attributes & 0x10) == 0x10) continue;

        // --- MUDANÇA AQUI ---
        // Em vez de só salvar o nome, salvamos o nome E o tamanho

        FileInfo info;
        info.name = formatFilename(entry);
        info.size = (long)entry.fileSize; // Já temos o tamanho aqui!

        fileList.push_back(info);
    }

    return fileList;
}

bool FAT16::convertToFAT83(const string& filename, char fatName[8], char fatExt[3]) {
    // 1. Preenche ambos com espaços (padrão da FAT)
    memset(fatName, ' ', 8);
    memset(fatExt, ' ', 3);

    size_t dotPos = filename.find('.');
    string namePart, extPart;

    if (dotPos == string::npos) {
        // Sem ponto (ex: "ARQUIVO")
        namePart = filename;
    } else {
        // Com ponto (ex: "TEXTO.TXT")
        namePart = filename.substr(0, dotPos);
        extPart = filename.substr(dotPos + 1);
    }

    // 2. Valida os tamanhos
    if (namePart.length() > 8 || extPart.length() > 3 || namePart.empty()) {
        return false; // Nome inválido
    }

    // 3. Copia e converte para maiúsculo
    for (size_t i = 0; i < namePart.length(); ++i) {
        fatName[i] = (char)std::toupper(namePart[i]);
    }
    for (size_t i = 0; i < extPart.length(); ++i) {
        fatExt[i] = (char)std::toupper(extPart[i]);
    }

    return true;
}


// (Esta é a implementação principal de renomear)
void FAT16::renameFile(const string& oldName, const string& newName) {
    if (!m_isMounted) {
        throw std::runtime_error("Sistema de arquivos nao montado.");
    }

    // 1. Encontrar o arquivo antigo
    DirectoryEntryFAT16 entry;
    long entryOffset = findRootEntry(oldName, entry); // (Esta função já existe no seu .cpp)

    if (entryOffset < 0) {
        throw std::runtime_error("Arquivo de origem nao encontrado.");
    }

    // 2. Verificar se o novo nome já existe
    DirectoryEntryFAT16 tempEntry;
    if (findRootEntry(newName, tempEntry) >= 0) {
        throw std::runtime_error("O novo nome de arquivo ja esta em uso.");
    }

    // 3. Converter o novo nome para o formato 8.3
    char newFatName[8];
    char newFatExt[3];
    if (!convertToFAT83(newName, newFatName, newFatExt)) {
        throw std::runtime_error("Novo nome de arquivo e invalido (deve ser 8.3).");
    }

    // 4. Preparar a operação de "Ler-Modificar-Escrever"

    // a. Calcular qual setor contém esta entrada
    uint32_t sectorIndex = entryOffset / m_bootSector.bytesPerSector;
    uint32_t offsetInSector = entryOffset % m_bootSector.bytesPerSector;

    // b. Ler o setor inteiro
    vector<uint8_t> sectorBuffer(m_bootSector.bytesPerSector);
    if (!m_osManager.readSectors(sectorIndex, 1, sectorBuffer.data())) {
        throw std::runtime_error("Falha ao ler o setor do diretorio.");
    }

    // c. Modificar a entrada de diretório NAQUELE SETOR
    //    (Primeiro, pegamos um ponteiro para a entrada *dentro* do buffer)
    auto* entryInSector = (DirectoryEntryFAT16*)(sectorBuffer.data() + offsetInSector);

    //    (Agora, copiamos o novo nome/extensão para ela)
    memcpy(entryInSector->fileName, newFatName, 8);
    memcpy(entryInSector->extension, newFatExt, 3);

    // d. Escrever o setor modificado de volta no disco
    if (!m_osManager.writeSectors(sectorIndex, 1, sectorBuffer.data())) {
        throw std::runtime_error("Falha ao escrever o setor do diretorio.");
    }
}

uint32_t FAT16::clusterToSector(uint16_t clusterIndex) {
    // Comeca no cluster 2.
    return m_dataStartSector + (uint32_t)(clusterIndex - 2) * m_bootSector.sectorsPerCluster;
}

bool FAT16::readCluster(uint16_t clusterIndex, uint8_t* buffer) {
    uint32_t sector = clusterToSector(clusterIndex);
    return m_osManager.readSectors(sector, m_bootSector.sectorsPerCluster, buffer);
}

vector<uint16_t> FAT16::getClusterChain(uint16_t firstCluster) {
    vector<uint16_t> chain;
    uint16_t currentCluster = firstCluster;

    // 0xFFF8 a 0xFFFF: são marcadores de "fim de arquivo" (EOC)
    // 0x0000: "cluster livre"
    // 0xFFF7: "cluster ruim"
    while (currentCluster >= 0x0002 && currentCluster < 0xFFF8) {
        chain.push_back(currentCluster);

        // Pega o próximo cluster do nosso cache
        currentCluster = m_fatCache[currentCluster];
    }

    return chain;
}

bool FAT16::findFileInRoot(const string& filename, DirectoryEntryFAT16& outEntry) {
    vector<uint8_t> rootDirBuffer(m_rootDirSectors * m_bootSector.bytesPerSector);
    if (!m_osManager.readSectors(m_rootDirStartSector, m_rootDirSectors, rootDirBuffer.data())) {
        return false;
    }

    auto* entries = (DirectoryEntryFAT16*)rootDirBuffer.data();
    for (uint32_t i = 0; i < m_bootSector.numRootDirEntries; i++) {
        DirectoryEntryFAT16& entry = entries[i];

        if (entry.fileName[0] == 0x00) break;
        if ((uint8_t)entry.fileName[0] == 0xE5) continue;
        if (entry.attributes == 0x0F) continue;

        if (formatFilename(entry) == filename) {
            outEntry = entry;
            return true;
        }
    }
    return false;
}

bool FAT16::readFile(const string& filename, vector<uint8_t>& outData) {
    outData.clear();
    if (!m_isMounted) {
        cerr << "Erro: Sistema nao montado." << endl;
        return false;
    }

    DirectoryEntryFAT16 entry;
    if (!findFileInRoot(filename, entry)) {
        cerr << "Erro: Arquivo '" << filename << "' nao encontrado." << endl;
        return false;
    }

    cout << "  [DEBUG] Arquivo encontrado: " << filename << endl;
    cout << "  [DEBUG] Tamanho (fileSize): " << entry.fileSize << " bytes" << endl;
    cout << "  [DEBUG] Cluster Inicial:    " << entry.firstCluster << endl;

    if (entry.fileSize == 0) {
        return true;
    }

    vector<uint16_t> clusterChain = getClusterChain(entry.firstCluster);
    if (clusterChain.empty()) {
        cerr << "Erro: Arquivo com tamanho > 0 mas sem clusters alocados." << endl;
        return false;
    }

    cout << "  [DEBUG] Cadeia de clusters encontrada. Tamanho: " << clusterChain.size() << " clusters." << endl;

    vector<uint8_t> clusterBuffer(m_bytesPerCluster);

    for (uint16_t clusterIndex : clusterChain) {
        if (!readCluster(clusterIndex, clusterBuffer.data())) {
            cerr << "Erro ao ler cluster " << clusterIndex << endl;
            outData.clear(); // Falha, limpa os dados
            return false;
        }

        uint32_t remainingSize = entry.fileSize - outData.size();
        uint32_t bytesToCopy = std::min(m_bytesPerCluster, remainingSize);

        outData.insert(outData.end(), clusterBuffer.begin(), clusterBuffer.begin() + bytesToCopy);

        if (outData.size() >= entry.fileSize) {
            break;
        }
    }

    return true;
}

long FAT16::findFreeRootEntry() {
    // (Lógica muito similar a findRootEntry, mas procura por 0x00 ou 0xE5)
    vector<uint8_t> rootDirBuffer(m_rootDirSectors * m_bootSector.bytesPerSector);
    if (!m_osManager.readSectors(m_rootDirStartSector, m_rootDirSectors, rootDirBuffer.data())) {
        throw std::runtime_error("Falha ao ler o diretorio raiz (em findFreeRootEntry).");
    }

    auto* entries = (DirectoryEntryFAT16*)rootDirBuffer.data();
    for (uint32_t i = 0; i < m_bootSector.numRootDirEntries; i++) {
        DirectoryEntryFAT16& entry = entries[i];
        uint8_t firstByte = (uint8_t)entry.fileName[0];

        // 0x00 (nunca usado) ou 0xE5 (deletado) são slots livres
        if (firstByte == 0x00 || firstByte == 0xE5) {
            long rootDirByteOffset = m_rootDirStartSector * m_bootSector.bytesPerSector;
            return rootDirByteOffset + (i * sizeof(DirectoryEntryFAT16)); // Encontrado!
        }
    }

    return -1; // Diretório Raiz Cheio
}

vector<uint16_t> FAT16::findFreeClusters(uint32_t count) {
    vector<uint16_t> freeClusters;

    // Começa em 2, pois 0 e 1 são reservados
    for (uint32_t i = 2; i < m_fatCache.size() && freeClusters.size() < count; ++i) {
        if (m_fatCache[i] == 0x0000) { // 0x0000 == Cluster Livre
            freeClusters.push_back(i);
        }
    }

    if (freeClusters.size() < count) {
        throw std::runtime_error("Disco cheio. Nao ha clusters livres suficientes.");
    }

    return freeClusters;
}

void FAT16::writeFatToDisk() {
    // Escreve o cache modificado de volta para TODAS as cópias da FAT
    for (uint32_t i = 0; i < m_bootSector.numFATs; ++i) {
        uint32_t fatSector = m_fatStartSector + (i * m_bootSector.sectorsPerFAT);
        if (!m_osManager.writeSectors(fatSector, m_bootSector.sectorsPerFAT, (uint8_t*)m_fatCache.data())) {
            throw std::runtime_error("Falha critica ao escrever na FAT no disco.");
        }
    }
}

void FAT16::writeCluster(uint16_t clusterIndex, const uint8_t* data, uint32_t size) {
    uint32_t sector = clusterToSector(clusterIndex); // (clusterToSector já existe)

    // Cria um buffer do tamanho de um cluster (ex: 4096 bytes), preenchido com 0
    vector<uint8_t> clusterBuffer(m_bytesPerCluster, 0);

    // Copia os dados do arquivo para o início do buffer
    memcpy(clusterBuffer.data(), data, size);

    // Escreve o cluster inteiro no disco
    if (!m_osManager.writeSectors(sector, m_bootSector.sectorsPerCluster, clusterBuffer.data())) {
        throw std::runtime_error("Falha ao escrever dados no cluster.");
    }
}

void FAT16::clearFatChain(uint16_t firstCluster) {
    if (firstCluster == 0) { // Arquivo de 0 bytes, sem clusters
        return;
    }

    uint16_t currentCluster = firstCluster;
    const uint16_t EOC = 0xFFF8; // Marcador de Fim de Cadeia

    // Itera pela cadeia de clusters
    while (currentCluster >= 0x0002 && currentCluster < EOC) {
        uint16_t nextCluster = m_fatCache[currentCluster];
        m_fatCache[currentCluster] = 0x0000; // Marca o cluster como livre
        currentCluster = nextCluster;
    }

    // Salva a FAT (agora limpa) de volta no disco
    writeFatToDisk(); // Esta função já existe (da implementação do writeFile)
}


// --- IMPLEMENTAÇÃO PRINCIPAL DE deleteFile ---
// (Adicione esta função junto com as outras funções públicas)

void FAT16::deleteFile(const string& filename) {
    if (!m_isMounted) {
        throw std::runtime_error("Sistema de arquivos nao montado.");
    }

    // 1. Encontrar a entrada de diretório do arquivo
    DirectoryEntryFAT16 entry;
    long entryOffset = findRootEntry(filename, entry); // (Esta função já existe)

    if (entryOffset < 0) {
        throw std::runtime_error("Arquivo nao encontrado.");
    }

    // 2. Limpar a cadeia de clusters do arquivo na Tabela FAT
    clearFatChain(entry.firstCluster);

    // 3. Marcar a entrada de diretório como "deletada" (0xE5)
    //    (Operação de "Ler-Modificar-Escrever")

    // a. Calcular qual setor contém esta entrada
    uint32_t sectorIndex = entryOffset / m_bootSector.bytesPerSector;
    uint32_t offsetInSector = entryOffset % m_bootSector.bytesPerSector;

    // b. Ler o setor inteiro
    vector<uint8_t> sectorBuffer(m_bootSector.bytesPerSector);
    if (!m_osManager.readSectors(sectorIndex, 1, sectorBuffer.data())) {
        throw std::runtime_error("Falha ao ler o setor do diretorio para apagar.");
    }

    // c. Modificar o primeiro byte da entrada NAQUELE SETOR
    //    (Pegamos um ponteiro para o primeiro byte do nome do arquivo)
    uint8_t* entryFirstByte = sectorBuffer.data() + offsetInSector;

    *entryFirstByte = 0xE5; // O marcador oficial de "deletado"

    // d. Escrever o setor modificado de volta no disco
    if (!m_osManager.writeSectors(sectorIndex, 1, sectorBuffer.data())) {
        throw std::runtime_error("Falha ao escrever o setor do diretorio para apagar.");
    }
}

// --- IMPLEMENTAÇÃO PRINCIPAL DE writeFile ---
// (Adicione esta função junto com as outras funções públicas)

void FAT16::writeFile(const string& newName, const vector<uint8_t>& data) {
    if (!m_isMounted) {
        throw std::runtime_error("Sistema de arquivos nao montado.");
    }

    DirectoryEntryFAT16 tempEntry;
    if (findRootEntry(newName, tempEntry) >= 0) {
        throw std::runtime_error("Um arquivo com este nome ja existe.");
    }

    char fatName[8];
    char fatExt[3];
    if (!convertToFAT83(newName, fatName, fatExt)) { // (convertToFAT83 já existe)
        throw std::runtime_error("Novo nome de arquivo e invalido (deve ser 8.3).");
    }

    // 2. Encontrar Espaço no Diretório Raiz
    long entryOffset = findFreeRootEntry();
    if (entryOffset < 0) {
        throw std::runtime_error("Diretorio raiz esta cheio.");
    }

    // 3. Encontrar Espaço de Dados (Clusters)
    // (Se o arquivo tiver 0 bytes, não precisamos de clusters)
    vector<uint16_t> clusterChain;
    uint16_t firstCluster = 0; // 0 significa "sem cluster" (para arquivos de 0 bytes)

    if (!data.empty()) {
        uint32_t numClusters = (data.size() + m_bytesPerCluster - 1) / m_bytesPerCluster;
        clusterChain = findFreeClusters(numClusters);
        firstCluster = clusterChain[0];

        // 4. Escrever os Dados nos Clusters
        for (size_t i = 0; i < clusterChain.size(); ++i) {
            const uint8_t* chunkData = data.data() + (i * m_bytesPerCluster);
            uint32_t remaining = data.size() - (i * m_bytesPerCluster);
            uint32_t chunkSize = std::min(m_bytesPerCluster, remaining);

            writeCluster(clusterChain[i], chunkData, chunkSize);
        }

        // 5. Atualizar a Tabela FAT em cache e no disco
        for (size_t i = 0; i < clusterChain.size() - 1; ++i) {
            m_fatCache[clusterChain[i]] = clusterChain[i + 1]; // Aponta para o próximo
        }
        m_fatCache[clusterChain.back()] = 0xFFFF; // Marcador de Fim de Arquivo (EOC)

        writeFatToDisk(); // Salva a FAT no disco
    }

    // 6. Escrever a Entrada de Diretório (Ler-Modificar-Escrever)
    uint32_t sectorIndex = entryOffset / m_bootSector.bytesPerSector;
    uint32_t offsetInSector = entryOffset % m_bootSector.bytesPerSector;

    vector<uint8_t> sectorBuffer(m_bootSector.bytesPerSector);
    if (!m_osManager.readSectors(sectorIndex, 1, sectorBuffer.data())) {
        throw std::runtime_error("Falha ao ler o setor do diretorio para escrever a entrada.");
    }

    auto* newEntry = (DirectoryEntryFAT16*)(sectorBuffer.data() + offsetInSector);

    // Zera a entrada (caso fosse uma 0xE5 com lixo)
    memset(newEntry, 0, sizeof(DirectoryEntryFAT16));

    // Preenche a entrada
    memcpy(newEntry->fileName, fatName, 8);
    memcpy(newEntry->extension, fatExt, 3);
    newEntry->attributes = 0x20; // 0x20 = Atributo de "Arquivo" (Archive bit)
    newEntry->fileSize = data.size();
    newEntry->firstCluster = firstCluster;
    // (Datas e horas de modificação ficariam aqui, mas pulamos para simplificar)

    if (!m_osManager.writeSectors(sectorIndex, 1, sectorBuffer.data())) {
        throw std::runtime_error("Falha ao escrever a entrada de diretorio no disco.");
    }

}