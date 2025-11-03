//
// Created by hb on 11/2/25.
//

#ifndef FAT16_OSMANAGER_H
#define FAT16_OSMANAGER_H
#include <cstdint>
#include <fstream>
#include <string>

using namespace std;

class OSManager {
public:
    ~OSManager();
    explicit OSManager(const string& path);
    bool readSectors(uint32_t sectorIndex, uint32_t count, uint8_t* buffer);
    bool writeSectors(uint32_t sectorIndex, uint32_t count, const uint8_t* buffer);
    bool isDiskOpen() const;
private:
    string m_path;
    fstream m_file;
    const uint32_t m_bytesPerSector = 512;
};

#endif //FAT16_OSMANAGER_H