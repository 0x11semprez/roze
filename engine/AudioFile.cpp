#include "interface/AudioFile.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>

using namespace std;

AudioFile::AudioFile(const string& filePath)
{
    setAudioFilePath(filePath);
    setAudioFileName(path(filePath).stem().string());
    setAudioFileFormat(path(filePath).extension().string());
}

void decodeFile(string name)
{
    ifstream file(name, ios::binary);
    if (!file) {
        cerr << "file not loaded";
    }

    array<char, 12> header{};

    if (!file.read(header.data(), header.size()) {
        cerr << "Cannot read";
    }

    for (char byte: header) {

    }
}
