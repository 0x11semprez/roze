#pragma once

#include <filesystem>
#include <string>

using namespace std;
using namespace filesystem;

class AudioFile {
    public:
    AudioFile(path p);  // path is no good;

    void setAudioFileName();
    void getAudioFileFormat();

    [[nodiscard]] string getAudioFilePath() const noexcept;
    [[nodiscard]] string getAudioFileName() const noexcept;
    [[nodiscard]] string getAudioFileFormat() const noexcept;

    private:
    path audioPath_;
    string name_;
    string audioFileFormat_;
};
