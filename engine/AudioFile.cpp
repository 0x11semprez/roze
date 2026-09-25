#include "interface/AudioFile.hpp"

#include <filesystem>
#include <fstream>
#include <ostream>

using namespace std;

AudioFile::AudioFile(const string& filePath)
{
    setAudioFilePath(filePath);
    setAudioFileName(path(filePath).stem().string());
    setAudioFileFormat(path(filePath).extension().string());
}

void AudioFile::setAudioFilePath(const string& filePath)
{
    audioPath_ = filePath;
}

void AudioFile::setAudioFileName(const string& name)
{
    name_ = name;
}

void AudioFile::setAudioFileFormat(const string& format)
{
    audioFileFormat_ = format;
}
