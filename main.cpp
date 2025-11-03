#include <iostream>

#include "DiskManager/OSManager.h"

using namespace std;

int main() {
    cout << "=== FAT16 OSManager Test ===" << endl;

    string path;
    cout << "Informe o caminho do arquivo: ";
    getline(cin, path);

    OSManager manager(path);

    while (true) {
        cout << "\n--- MENU ---" << endl;
        cout << "1. Definir novo caminho" << endl;
        cout << "2. Escrever no arquivo" << endl;
        cout << "3. Ler arquivo" << endl;
        cout << "4. Sair" << endl;
        cout << "Escolha: ";

        int opcao;
        cin >> opcao;
        cin.ignore(); // limpa \n pendente

        if (opcao == 1) {
            cout << "Novo caminho: ";
            getline(cin, path);
            manager.setPath(path);
        }
        else if (opcao == 2) {
            cout << "Digite o texto para salvar no arquivo:" << endl;
            string texto;
            getline(cin, texto);

            vector<char> data(texto.size());
            for (size_t i = 0; i < texto.size(); ++i)
                data[i] = static_cast<vector<char>::value_type>(static_cast<std::byte>(texto[i]));

            manager.writeFile(data);
            cout << "✅ Arquivo salvo com sucesso!" << endl;
        }
        else if (opcao == 3) {
            cout << "📂 Conteúdo do arquivo: " << endl;

            // Força releitura do arquivo
            manager.setPath(path);

            for (auto b : manager.getData())
                cout << static_cast<char>(b);
            cout << endl;
        }
        else if (opcao == 4) {
            cout << "Saindo..." << endl;
            break;
        }
        else {
            cout << "Opção inválida!" << endl;
        }
    }

    return 0;
}
