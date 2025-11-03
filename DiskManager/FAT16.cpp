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

vector<string> FAT16::listRootDirectory() {
    vector<string> fileList;
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

        if (entry.fileName[0] == 0x00) {
            break;
        }

        if ((uint8_t)entry.fileName[0] == 0xE5) { //arquivo deletado -> pula
            continue;
        }

        if (entry.attributes == 0x0F) {
            continue;
        }

        if ((entry.attributes & 0x10) == 0x10) { //subdiretorio
            continue;
        }

        fileList.push_back(formatFilename(entry));
    }

    return fileList;
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