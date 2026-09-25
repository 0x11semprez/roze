#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

using namespace std;
using namespace filesystem;

class AudioFile {
    public:
    AudioFile(const string& filePath);

    void setAudioFilePath(const string& filePath)
    {
        audioPath_ = filePath;
    }

    void setAudioFileName(const string& name)
    {
        name_ = name;
    }
    void setAudioFileFormat(const string& format)
    {
        audioFileFormat_ = format;
    }

    [[nodiscard]] bool load(const string& filePath);

    [[nodiscard]] const string& getAudioFilePath() const noexcept
    {
        return audioPath_;
    }
    [[nodiscard]] const string& getAudioFileName() const noexcept
    {
        return name_;
    }
    [[nodiscard]] const string& getAudioFileFormat() const noexcept
    {
        return audioFileFormat_;
    }
    [[nodiscard]] bool isLoaded() const noexcept
    {
        return isLoaded_;
    }
    [[nodiscard]] uint32_t getSampleRate() const noexcept
    {
        return sampleRate_;
    }
    [[nodiscard]] int getNumChannels() const noexcept
    {
        return numChannels_;
    }
    [[nodiscard]] bool isMono() const noexcept
    {
        return isMono_;
    }
    [[nodiscard]] bool isStereo() const noexcept
    {
        return isStereo_;
    }
    [[nodiscard]] int getBitDepth() const noexcept
    {
        return bitDepth_;
    }

    private:
    string audioPath_;
    string name_;
    string audioFileFormat_;
    bool isLoaded_ = false;
    uint32_t sampleRate_ = 0;
    int numChannels_ = 0;
    bool isMono_ = false;
    bool isStereo_ = false;
    int bitDepth_ = 0;
};
