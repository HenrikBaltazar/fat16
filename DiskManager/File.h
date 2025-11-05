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
    const std::string& getPath() const {
        return m_fatSystem.getPath();
    }

    bool getIsMounted();
    vector<FileInfo> listRootDirectory();
    vector<uint8_t> readFile(const string& filename);
    long getFileSize(const string& filename);
    vector<FileInfo> getFilesInfo();
    void renameFile(const string& oldName, const string& newName);
    void loadFiles();
    void writeFile(const string& hostPath, const string& fatName);
    void deleteFile(const string& filename);
    static std::string formatFatDate(uint16_t date);
    static std::string formatFatTime(uint16_t time);

private:
    vector<FileInfo> files;
    FAT16 m_fatSystem;
};

#endif //FAT16_FILE_H