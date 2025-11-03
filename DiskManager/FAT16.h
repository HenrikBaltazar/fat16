//
// Created by hb on 11/2/25.
//

#ifndef FAT16_FAT16_H
#define FAT16_FAT16_H
#include <cstdint>

struct BootSectorFAT16 {
    uint8_t     jumpCode[3];
    char        oemName[8];
    uint16_t    bytesPerSector;      // Bytes por setor (Ex: 512)
    uint8_t     sectorsPerCluster;   // Setores por "cluster" (unidade de alocação)
    uint16_t    reservedSectors;     // Setores antes da primeira FAT (normalmente 1, o próprio VBR)
    uint8_t     numFATs;             // Número de FATs (normalmente 2)
    uint16_t    numRootDirEntries;   // Número de entradas no diretório raiz
    uint16_t    totalSectorsSmall;   // Número total de setores (se < 65536)
    uint8_t     mediaDescriptor;
    uint16_t    sectorsPerFAT;       // Tamanho de cada FAT em setores
    uint16_t    sectorsPerTrack;
    uint16_t    numHeads;
    uint32_t    hiddenSectors;
    uint32_t    totalSectorsLarge;   // Número total de setores (se > 65536)

    // Campos extendidos do BPB (BIOS Parameter Block)
    uint8_t     driveNumber;
    uint8_t     _reserved;
    uint8_t     bootSignature;       // Assinatura (0x29)
    uint32_t    volumeID;
    char        volumeLabel[11];
    char        fileSystemType[8];   // "FAT16   "

    // O resto é código de boot e a assinatura 0xAA55 no final
    uint8_t     bootCode[448];
    uint16_t    bootableSignature;   // 0xAA55

};

class FAT16 {
//TODO:
};


#endif //FAT16_FAT16_H