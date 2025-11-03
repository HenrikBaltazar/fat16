#include <iostream>

#include "DiskManager/OSManager.h"

#include <iostream>
#include <string>
#include <vector>

#include "DiskManager/FAT16.h"

using namespace std;

void printFileContent(const vector<uint8_t>& data) {
// TODO: Mudar para a classe FILE
    cout << "--- Conteudo do Arquivo  ---" << endl;

    string content(data.begin(), data.end());


    cout << content << endl;
    cout << "------------------------------------------------" << endl;
    cout << "Tamanho total lido: " << data.size() << " bytes." << endl;
}


int main() {
    string diskImagePath = "disco2.img";

    cout << "Abrindo imagem de disco: " << diskImagePath << endl;
    OSManager osManager(diskImagePath);

    if (!osManager.isDiskOpen()) {
        cerr << "Falha ao abrir a imagem de disco. Encerrando." << endl;
        return 1;
    }

    FAT16 fatSystem(osManager);

    cout << "\nTentando montar o sistema de arquivos FAT16..." << endl;

    if (!fatSystem.mount()) {
        cerr << "Falha ao montar o sistema FAT16. Encerrando." << endl;
        return 1;
    }

    cout << "\n--- Listando Arquivos no Diretorio Raiz ---" << endl;
    vector<string> files = fatSystem.listRootDirectory();

    if (files.empty()) {
        cout << "Nenhum arquivo encontrado no diretorio raiz." << endl;
    } else {
        for (const string& filename : files) {
            cout << "- " << filename << endl;
        }
    }
    cout << "-------------------------------------------" << endl;


    string fileToRead = "TEXTO2.TXT";

    vector<uint8_t> fileData;

    cout << "\nTentando ler o arquivo: " << fileToRead << "..." << endl;
    if (fatSystem.readFile(fileToRead, fileData)) {
        cout << "Arquivo lido com sucesso!" << endl;
        printFileContent(fileData);
    } else {
        cerr << "Falha ao ler o arquivo: " << fileToRead << endl;
    }


    cout << "\nTeste concluido." << endl;
    return 0;
}