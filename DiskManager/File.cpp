//
// Created by hb on 11/3/25.
//

#include "File.h"
#include <iostream>
#include <stdexcept>

File::File(const string& diskImagePath) : m_fatSystem(diskImagePath) {
    m_fatSystem.mount();
    files = listRootDirectory();
}

void File::loadFiles() {
    files.clear();
    files = listRootDirectory();
}

void File::writeFile(const string& hostPath, const string& fatName) {
    if (!m_fatSystem.getIsMounted()) {
        throw std::runtime_error("Sistema de arquivos nao montado.");
    }

    // 1. Ler o arquivo do sistema host
    ifstream hostFile(hostPath, ios::binary | ios::ate); // Abre no final
    if (!hostFile) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo do host: " + hostPath);
    }

    streamsize size = hostFile.tellg(); // Pega o tamanho
    hostFile.seekg(0, ios::beg);        // Volta para o início

    vector<uint8_t> data(size);
    if (!hostFile.read((char*)data.data(), size)) {
        throw std::runtime_error("Falha ao ler os dados do arquivo do host.");
    }
    hostFile.close();

    m_fatSystem.writeFile(fatName, data);
    loadFiles();
}

vector<FileInfo> File::getFilesInfo() {
    return files;
}

void File::renameFile(const string& oldName, const string& newName) {
    if (!m_fatSystem.getIsMounted()) {
        throw std::runtime_error("Sistema de arquivos nao montado.");
    }

    m_fatSystem.renameFile(oldName, newName);
}

vector<FileInfo> File::listRootDirectory() {
    if (!m_fatSystem.getIsMounted()) {
        cerr << "Erro: Sistema nao montado." << endl;
        return {}; // Retorna vetor vazio
    }
    // Agora só repassa o vetor de FileInfo
    return m_fatSystem.listRootDirectory();
}

bool File::getIsMounted() {
    return m_fatSystem.getIsMounted();
}

long File::getFileSize(const string& filename) {
    if (!m_fatSystem.getIsMounted()) {
        cerr << "Erro: Sistema nao montado." << endl;
        return -1;
    }
    return m_fatSystem.getFileSize(filename);
}

vector<uint8_t> File::readFile(const string& filename) {
    vector<uint8_t> data;
    if (!m_fatSystem.readFile(filename, data)) {
        data.clear();
    }
    return data;
}

void File::deleteFile(const string& filename) {
    if (!m_fatSystem.getIsMounted()) {
        throw std::runtime_error("Sistema de arquivos nao montado.");
    }

    // Repassa a chamada para a lógica do FAT16
    m_fatSystem.deleteFile(filename);
    loadFiles();
}