//
// Created by hb on 11/3/25.
//

#include "File.h"
#include <iostream>

File::File(const string& diskImagePath) : m_fatSystem(diskImagePath) {
    m_fatSystem.mount();
}

vector<string> File::listRootDirectory() {
    return m_fatSystem.listRootDirectory();
}

bool File::getIsMounted() {
    return m_fatSystem.getIsMounted();
}

vector<uint8_t> File::readFile(const string& filename) {
    vector<uint8_t> data;
    if (!m_fatSystem.readFile(filename, data)) {
        data.clear();
    }
    return data;
}