//
// Created by hb on 11/3/25.
//

#ifndef FAT16_FILE_H
#define FAT16_FILE_H
#include <string>
#include <vector>

#include "FAT16.h"

using namespace std;

class File {
public:
    explicit File(const string& diskImagePath);

    bool getIsMounted();
    vector<string> listRootDirectory();
    vector<uint8_t> readFile(const string& filename);

private:
    FAT16 m_fatSystem;
};

#endif //FAT16_FILE_H