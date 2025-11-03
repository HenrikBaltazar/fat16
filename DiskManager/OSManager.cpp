//
// Created by hb on 11/2/25.
//

#include "OSManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;

OSManager::OSManager(const string& path)
{
    file.path = path;
    readFile();
}

void OSManager::readFile() {
    ifstream in(file.path, ios_base::binary);
    if (!in) {
        cerr << "Erro ao abrir arquivo para leitura: " << file.path << endl;
        return;
    }
    file.data.assign(istreambuf_iterator<char>(in), {});
    file.name = std::filesystem::path(file.path).filename().string();
}

void OSManager::setPath(const string &path) {
    file.path = path;
    readFile();
}

void OSManager::writeFile(const vector<char>& data) {
    ofstream out(file.path, ios::binary);
    if (!out) {
        cerr << "Erro ao abrir arquivo para escrita: " << file.path << endl;
        return;
    }

    for (auto b : data)
        out.put(static_cast<char>(b));
}

vector<char> OSManager::getData() {
    return file.data;
}

