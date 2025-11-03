//
// Created by hb on 11/2/25.
//

#include "OSManager.h"
#include <iostream>

OSManager::OSManager(const string& path) : m_path(path) {

    m_file.open(m_path, ios::in | ios::out | ios::binary);

    if (!m_file.is_open()) {
        cerr << "Nao foi possivel abrir o arquivo: " << m_path << endl;
    }
}

OSManager::~OSManager() {
    if (m_file.is_open()) {
        m_file.close();
    }
}


bool OSManager::isDiskOpen() const {
    return m_file.is_open();
}

bool OSManager::readSectors(uint32_t sectorIndex, uint32_t count, uint8_t* buffer) {
    if (!isDiskOpen()) {
        cerr << "Erro:leitura de disco fechado." << endl;
        return false;
    }

    uint64_t offset = (uint64_t)sectorIndex * m_bytesPerSector;

    m_file.seekg(offset);

    m_file.read(reinterpret_cast<char*>(buffer), (uint64_t)count * m_bytesPerSector);

    if (m_file.fail()) {
        cerr << "Erro ao ler " << count << " setores do indice " << sectorIndex << endl;
        return false;
    }

    return true;
}


bool OSManager::writeSectors(uint32_t sectorIndex, uint32_t count, const uint8_t* buffer) {
    if (!isDiskOpen()) {
        cerr << "Erro: escrita de disco fechado." << endl;
        return false;
    }

    uint64_t offset = (uint64_t)sectorIndex * m_bytesPerSector;

    m_file.seekp(offset);

    m_file.write(reinterpret_cast<const char*>(buffer), (uint64_t)count * m_bytesPerSector);

    if (m_file.fail()) {
        cerr << "Erro ao escrever " << count << " setores no indice " << sectorIndex << endl;
        return false;
    }

    return true;
}