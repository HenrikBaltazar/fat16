//
// Created by hb on 11/2/25.
//

#ifndef FAT16_FAT16_H
#define FAT16_FAT16_H
#include <cstdint>
#include <string>
#include <vector>
#include "OSManager.h"

using namespace std;

#pragma pack(push, 1) //previne o alinhamento do gpp


struct BootSectorFAT16 {
    uint8_t     jumpCode[3];
    char        oemName[8];
    uint16_t    bytesPerSector;      // 512, 1024, 2048, 4096
    uint8_t     sectorsPerCluster;   // 1, 2, 4, 8, 16, 32, 64, 128
    uint16_t    reservedSectors;
    uint8_t     numFATs;
    uint16_t    numRootDirEntries;
    uint16_t    totalSectorsSmall;
    uint8_t     mediaDescriptor;
    uint16_t    sectorsPerFAT;
    uint16_t    sectorsPerTrack;
    uint16_t    numHeads;
    uint32_t    hiddenSectors;
    uint32_t    totalSectorsLarge;

    uint8_t     driveNumber;
    uint8_t     _reserved;
    uint8_t     bootSignature;       // 0x29
    uint32_t    volumeID;
    char        volumeLabel[11];
    char        fileSystemType[8];   // Deve ser "FAT16   "
    uint8_t     bootCode[448];
    uint16_t    bootableSignature;   // 0xAA55
};


struct DirectoryEntryFAT16 {
    char        fileName[8];
    char        extension[3];
    uint8_t     attributes;
    uint8_t     _reserved[10];
    uint16_t    modificationTime;
    uint16_t    modificationDate;
    uint16_t    firstCluster;
    uint32_t    fileSize;
};
#pragma pack(pop)

struct FileInfo {
    string name;
    long size;
};

class FAT16 {
public:
    explicit FAT16(const string& diskImagePath);

    bool mount();
    vector<FileInfo> listRootDirectory();

    bool readFile(const string& filename, vector<uint8_t>& outData);
    void writeFile(const string& newName, const vector<uint8_t>& data);
    bool getIsMounted();
    long findRootEntry(const string& filename, DirectoryEntryFAT16& outEntry);
    long getFileSize(const string& filename);
    void renameFile(const string& oldName, const string& newName);
    void deleteFile(const string& filename);
private:
    void clearFatChain(uint16_t firstCluster);
    string formatFilename(const DirectoryEntryFAT16& entry);
    vector<uint16_t> getClusterChain(uint16_t firstCluster);
    bool readCluster(uint16_t clusterIndex, uint8_t* buffer);
    uint32_t clusterToSector(uint16_t clusterIndex);
    bool findFileInRoot(const string& filename, DirectoryEntryFAT16& outEntry);
    bool convertToFAT83(const string& filename, char fatName[8], char fatExt[3]);
    long findFreeRootEntry();
    vector<uint16_t> findFreeClusters(uint32_t count);
    void writeFatToDisk();
    void writeCluster(uint16_t clusterIndex, const uint8_t* data, uint32_t size);

    OSManager m_osManager;
    bool m_isMounted;

    BootSectorFAT16 m_bootSector;
    uint32_t m_fatStartSector;       // Setor onde a 1ª FAT começa
    uint32_t m_rootDirStartSector;   // Setor onde o diretório raiz começa
    uint32_t m_dataStartSector;      // Setor onde os dados (clusters) começam
    uint32_t m_bytesPerCluster;
    uint32_t m_rootDirSectors;       // Quantos setores o diretório raiz ocupa

    vector<uint16_t> m_fatCache;
};

#endif //FAT16_FAT16_H