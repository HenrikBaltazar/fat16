//
// Created by hb on 11/2/25.
//

#ifndef FAT16_OSMANAGER_H
#define FAT16_OSMANAGER_H
#include <string>
#include <vector>

using namespace std;


struct File{
    string path, name;
    vector<char> data;
};

class OSManager {
public:
    explicit OSManager(const string& path);

    void writeFile(const vector<char>& data);
    void setPath(const string& path);
    vector<char> getData();

    private:
    void readFile();

    File file;
};

#endif //FAT16_OSMANAGER_H