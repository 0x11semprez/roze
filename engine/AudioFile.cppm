module;

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

export module roze.audiofile;

export import roze.audiofile.errors;
import roze.registry;

export class AudioFile {
    public:
    AudioFile(const std::string& filePath)
    {
        setAudioFilePath(filePath);
        setAudioFileName(std::filesystem::path(filePath).stem().string());
        setAudioFileFormat(std::filesystem::path(filePath).extension().string());
    }

    void setAudioFilePath(const std::string& filePath)
    {
        audioPath_ = filePath;
    }

    void setAudioFileName(const std::string& name)
    {
        name_ = name;
    }
    void setAudioFileFormat(const std::string& format)
    {
        audioFileFormat_ = format;
    }

    // Reads the file header through the registry. Returns nullopt on success.
    [[nodiscard]] std::optional<AudioFileErrors> load(const std::string& filePath)
    {
        setAudioFilePath(filePath);
        setAudioFileName(std::filesystem::path(filePath).stem().string());
        setAudioFileFormat(std::filesystem::path(filePath).extension().string());

        isLoaded_ = false;

        AudioInfo info;
        if (const auto error = RozeAudioRegistry::decode(audioFileFormat_, filePath, info)) {
            return error;
        }

        sampleRate_ = info.sampleRate;
        numChannels_ = info.numChannels;
        bitDepth_ = info.bitDepth;
        isMono_ = numChannels_ == 1;
        isStereo_ = numChannels_ == 2;
        isLoaded_ = true;
        return std::nullopt;
    }

    [[nodiscard]] const std::string& getAudioFilePath() const noexcept
    {
        return audioPath_;
    }
    [[nodiscard]] const std::string& getAudioFileName() const noexcept
    {
        return name_;
    }
    [[nodiscard]] const std::string& getAudioFileFormat() const noexcept
    {
        return audioFileFormat_;
    }
    [[nodiscard]] bool isLoaded() const noexcept
    {
        return isLoaded_;
    }
    [[nodiscard]] std::uint32_t getSampleRate() const noexcept
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
    std::string audioPath_;
    std::string name_;
    std::string audioFileFormat_;
    bool isLoaded_ = false;
    std::uint32_t sampleRate_ = 0;
    int numChannels_ = 0;
    bool isMono_ = false;
    bool isStereo_ = false;
    int bitDepth_ = 0;
};
