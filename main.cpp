#include <iostream>
#include <string>
#include <vector>

// UNICO INCLUDE NECESSÁRIO
#include <cstdint>

#include "DiskManager/File.h"

using namespace std;

// A função printFileContent continua igual
void printFileContent(const vector<uint8_t>& data) {
    cout << "--- Conteudo do Arquivo  ---" << endl;
    string content(data.begin(), data.end());
    cout << content << endl;
    cout << "------------------------------------------------" << endl;
    cout << "Tamanho total lido: " << data.size() << " bytes." << endl;
}


int main() {
    string diskImagePath = "disco2.img"; // Use "disco1.img" ou "disco2.img"
    cout << "Abrindo imagem de disco: " << diskImagePath << endl;

    File diskFile(diskImagePath);

    cout << "\n--- Listando Arquivos no Diretorio Raiz ---" << endl;
    vector<string> files = diskFile.listRootDirectory();

    if (files.empty()) {
        cout << "Nenhum arquivo encontrado no diretorio raiz." << endl;
    } else {
        for (const string& filename : files) {
            cout << "- " << filename << endl;
        }
    }
    cout << "-------------------------------------------" << endl;

    string fileToRead = "TESTE.TXT";
    cout << "\nTentando ler o arquivo: " << fileToRead << "..." << endl;

    vector<uint8_t> fileData = diskFile.readFile(fileToRead);

    if (fileData.empty()) {
        cerr << "Falha ao ler o arquivo ou arquivo esta vazio: " << fileToRead << endl;
    } else {
        cout << "Arquivo lido com sucesso!" << endl;
        printFileContent(fileData);
    }

    cout << "\nTeste concluido." << endl;
    return 0;
}