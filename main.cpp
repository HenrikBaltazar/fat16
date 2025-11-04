#include <iostream>
#include <string>
#include <vector>

// UNICO INCLUDE NECESSÁRIO
#include <algorithm>
#include <cstdint>
#include <optional>
#include <filesystem>

#include "DiskManager/File.h"

using namespace std;

void printFileContent(const vector<uint8_t>& data) {
    cout << "--- Conteudo do Arquivo  ---" << endl;
    string content(data.begin(), data.end());
    cout << content << endl;
    cout << "------------------------------------------------" << endl;
    cout << "Tamanho total lido: " << data.size() << " bytes." << endl;
}

void listRootDirectory(optional<File>& diskFile) {
    cout << "\n--- Listando Arquivos no Diretorio Raiz ---" << endl;

    if (diskFile->getFilesInfo().empty()) {
        cout << "Nenhum arquivo encontrado no diretorio raiz." << endl;
    } else {
        for (const FileInfo& file : diskFile->getFilesInfo()) {
            cout << "- " << file.name << ": " << file.size << " bytes" << endl;
        }
    }
    cout << "-------------------------------------------" << endl;
}

string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c) { return std::toupper(c); });
    return str;
}

void showFileContent(optional<File>& diskFile) {
    cout << "Nome do arquivo: ";
    string filename;
    cin >> filename;

    filename = toUpper(filename);

    try {
        vector<uint8_t> data = diskFile->readFile(filename);
        printFileContent(data);
    } catch (const exception& e) {
        cerr << "Erro ao ler o arquivo: " << e.what() << endl;
    }
}

void showFileAttributes(optional<File>& diskFile) {
    if (!diskFile.has_value()) {
        cerr << "Erro: Nenhum disco montado." << endl;
        return;
    }

    cout << "Nome do arquivo (8.3): ";
    string filename;
    cin >> filename;

    filename = toUpper(filename);

    // Obtém a lista de arquivos do diretório raiz
    vector<FileInfo> files = diskFile->getFilesInfo();

    // Procura o arquivo pelo nome
    auto it = find_if(files.begin(), files.end(), [&](const FileInfo& f) {
        return f.name == filename;
    });

    if (it != files.end()) {
        cout << "--- Informações do Arquivo ---" << endl;
        cout << "Nome: " << it->name << endl;
        cout << "Tamanho: " << it->size << " bytes" << endl;
        cout << "--------------------------------" << endl;
    } else {
        cerr << "Erro: Arquivo '" << filename << "' não encontrado no diretório raiz." << endl;
    }
}

void renameFile(optional<File>& diskFile) {
    string oldName, newName;
    cout << "Nome do arquivo atual: ";
    cin >> oldName;
    cout << "Novo nome do arquivo: ";
    cin >> newName;

    oldName = toUpper(oldName);
    newName = toUpper(newName);
    try {
        diskFile->renameFile(oldName, newName);
        cout << "Arquivo renomeado com sucesso!" << endl;
        diskFile->loadFiles();
    } catch (const exception& e) {
        cerr << "Erro ao renomear: " << e.what() << endl;
    }
}

void deleteFile(optional<File>& diskFile) {
    string filename;
    cout << "Nome do arquivo a ser apagado: ";
    cin >> filename;

    filename = toUpper(filename); // Usando sua função helper

    try {
        diskFile->deleteFile(filename);
        cout << "Arquivo '" << filename << "' apagado com sucesso." << endl;
    } catch (const exception& e) {
        cerr << "Erro ao apagar arquivo: " << e.what() << endl;
    }
}

void addFile(optional<File>& diskFile) {
    string hostPath, fatName;
    cout << "Caminho do arquivo no seu PC: ";
    cin >> hostPath;
    cout << "Nome do arquivo no disco: ";
    cin >> fatName;

    fatName = toUpper(fatName);

    try {
        diskFile->writeFile(hostPath, fatName);
        cout << "Arquivo '" << hostPath << "' escrito com sucesso como '"
             << fatName << "' no disco." << endl;
    } catch (const exception& e) {
        cerr << "Erro ao escrever arquivo: " << e.what() << endl;
    }
}

int main() {
    string filename;
    bool isValidFile = false, run = true;
    optional<File> diskFile;

    while (run) {
        cout << " -- FAT16 Disk Manager -- " << endl;
        if (!isValidFile)
            while (true) {
                cout << "Insira um arquivo .img: ";
                cin >> filename;
                if (filesystem::exists(filename) && filesystem::is_regular_file(filename)) {
                    cout << "Abrindo imagem de disco: " << filename << endl;
                    diskFile.emplace(filename);
                    isValidFile = true;
                    break;
                }
            }

        if (!diskFile) {
            cerr << "Erro interno: arquivo não carregado corretamente." << endl;
            break;
        }

        int option;
        cin.ignore();
        cout << " -- Selecione uma das opcoes abaixo -- " << endl;
        cout << "1 -> Listar conteudo do disco" << endl;
        cout << "2 -> Listar o conteudo de um arquivo" << endl;
        cout << "3 -> Exibir os atributos de um arquivo" << endl;
        cout << "4 -> Renomear um arquivo" << endl;
        cout << "5 -> inserir um arquivo ao disco" << endl;
        cout << "6 -> Apagar um arquivo" << endl;
        cout << "7 -> Trocar imagem de disco" << endl;
        cout << "8 -> Sair" << endl;
        cout << "Opcao: ";
        cin >> option;

        switch (option) {
            case 1:
                listRootDirectory(diskFile);
                break;
            case 2:
                showFileContent(diskFile);
                break;
            case 3:
                showFileAttributes(diskFile);
                break;
            case 4:
                renameFile(diskFile);
                break;
            case 5:
                addFile(diskFile);
                break;
            case 6:
                deleteFile(diskFile);
                break;
            case 7:
                isValidFile = false;
                break;
            case 8:
                run = false;
                break;
        }

    }


    return 0;
}