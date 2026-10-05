module;

#include <string_view>

export module roze.audiofile.errors;

export enum class AudioFileErrors {
    AudioFileNotLoaded,
    CannotOpenFile,
    CannotReadHeader,
    NotRiffFile,
    NotWaveFile,
    FmtChunkNotFound,
    DataChunkNotFound,
    UnsupportedEncoding,
    InvalidChannelCount,
    InvalidSampleRate,
    InvalidBitDepth,
    UnsupportedFormat,
    FormatHasNoHeader,
    EncryptedFile,
    ProprietaryFormat,
    NotAnAudioFile,
    UnknownCodec,
    HeaderDoesNotMatchExtension,
};

export std::string_view errorMessage(AudioFileErrors error)
{
    switch (error) {
        case AudioFileErrors::AudioFileNotLoaded:
            return "roze could not load this audio file";
        case AudioFileErrors::CannotOpenFile:
            return "roze cannot open this file, check the path";
        case AudioFileErrors::CannotReadHeader:
            return "roze cannot read the header, the file is too short or broken";
        case AudioFileErrors::NotRiffFile:
            return "roze expected a RIFF file, this is not one";
        case AudioFileErrors::NotWaveFile:
            return "roze expected a WAVE file, this is not one";
        case AudioFileErrors::FmtChunkNotFound:
            return "roze found no 'fmt ' chunk in this wav file";
        case AudioFileErrors::DataChunkNotFound:
            return "roze found no 'data' chunk in this wav file";
        case AudioFileErrors::UnsupportedEncoding:
            return "roze only decodes PCM and float wav files";
        case AudioFileErrors::InvalidChannelCount:
            return "roze found an invalid number of channels";
        case AudioFileErrors::InvalidSampleRate:
            return "roze found an invalid sample rate";
        case AudioFileErrors::InvalidBitDepth:
            return "roze found an invalid bit depth";
        case AudioFileErrors::UnsupportedFormat:
            return "roze loves you, but cannot decode this format yet";
        case AudioFileErrors::FormatHasNoHeader:
            return "roze cannot read info from this format, it has no header";
        case AudioFileErrors::EncryptedFile:
            return "roze cannot read this file, it is encrypted";
        case AudioFileErrors::ProprietaryFormat:
            return "roze cannot read this proprietary format";
        case AudioFileErrors::NotAnAudioFile:
            return "roze looked inside, this is not an audio file";
        case AudioFileErrors::UnknownCodec:
            return "roze does not know the codec used in this file";
        case AudioFileErrors::HeaderDoesNotMatchExtension:
            return "roze read the header, it does not match the file extension";
        default:
            return "roze don't understand what happened to this audio file";
    }
}
