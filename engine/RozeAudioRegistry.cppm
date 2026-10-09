module;

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

export module roze.registry;

export import roze.audiofile.errors;
export import roze.registry.errors;

using namespace std;
using namespace std::literals;

export struct AudioInfo {
    uint32_t sampleRate = 0;
    int numChannels = 0;
    int bitDepth = 0;  // 0 = lossy codec
};

export class RozeAudioRegistry {
    public:
    static constexpr array<string_view, 47> allFormats{
        ".3gp",  ".aa",     ".aac", ".aax",  ".act",  ".aiff",  ".alac", ".amr", ".ape", ".au",
        ".awb",  ".dss",    ".dvf", ".flac", ".gsm",  ".iklax", ".ivs",  ".m4a", ".m4b", ".m4p",
        ".mmf",  ".movpkg", ".mp1", ".mp2",  ".mp3",  ".mpc",   ".msv",  ".nmf", ".ogg", ".oga",
        ".mogg", ".opus",   ".qoa", ".ra",   ".rm",   ".raw",   ".rf64", ".sln", ".tta", ".voc",
        ".vox",  ".wav",    ".wma", ".wv",   ".webm", ".8svx",  ".cda"};

    [[nodiscard]] static int allFormatsSupportbyRoze() noexcept
    {
        return static_cast<int>(allFormats.size());
    }

    [[nodiscard]] static string_view isSupported(string_view format)
    {
        for (const auto& f : allFormats) {
            if (format == f) {
                return "roze support your format.";
            }
        }
        return errorMessage(RozeAudioRegistryErrors::FormatIsNotSupported);
    }

    // Reads the header of `filePath` for extension `format` (".wav", ".flac", ...) and fills
    // `info`.
    [[nodiscard]] static optional<AudioFileErrors> decode(string_view format,
                                                          const string& filePath, AudioInfo& info);
};

// How to read one value. Le = little-endian (small byte first), Be = big-endian (big byte first).
enum class Read { Constant, Byte, Le16, Le32, Be16, Be24, Be32, FloatBe32, FloatBe64, Extended80 };

// One value: read `read` at `offset` bytes after the anchor,
// then (raw >> shift) & mask, then table[value] if there is a table, then + add.
struct Field {
    Read read = Read::Constant;
    uint32_t offset = 0;
    uint32_t value = 0;  // Read::Constant only
    int shift = 0;
    uint32_t mask = 0;  // 0 = no mask
    int add = 0;
    span<const uint32_t> table{};
    string_view anchor{};  // own anchor instead of the layout anchor
};

// Bytes that must be at a fixed offset, for example "RIFF" at 0. `mask` keeps only some bits.
struct Check {
    uint32_t offset = 0;
    string_view bytes{};
    string_view mask{};
};

enum class Kind { Header, NoHeader, Proprietary };

// Where the info of one format is. A format can have several layouts (MPEG 1, MPEG 2...).
// checks: bytes the file must have, else HeaderDoesNotMatchExtension.
// anchor: bytes searched in the file ("fmt ", "fLaC"...). Field offsets start at the anchor.
struct Layout {
    string_view extension;
    Kind kind = Kind::Header;
    array<Check, 3> checks{};
    string_view anchor{};
    string_view anchorMask{};
    Field sampleRate{};
    Field numChannels{};
    Field bitDepth{};
};

constexpr array<uint32_t, 3> mpeg1Rates{44100, 48000, 32000};
constexpr array<uint32_t, 3> mpeg2Rates{22050, 24000, 16000};
constexpr array<uint32_t, 3> mpeg25Rates{11025, 12000, 8000};
constexpr array<uint32_t, 4> mpegChannels{2, 2, 2, 1};  // channel mode 3 = mono
constexpr array<uint32_t, 13> aacRates{96000, 88200, 64000, 48000, 44100, 32000, 24000,
                                       22050, 16000, 12000, 11025, 8000,  7350};
constexpr array<uint32_t, 8> auBits{0, 0, 8, 16, 24, 32, 32, 64};  // encoding 1 = mu-law

constexpr Field constant(uint32_t value)
{
    return {.read = Read::Constant, .value = value};
}

// MP4 (.3gp .aax .alac .m4a .m4b .m4p): offsets from the "stsd" box type to its first entry.
constexpr uint32_t mp4Channels = 36;
constexpr uint32_t mp4SampleSize = 38;
constexpr uint32_t mp4SampleRate = 44;

// Sources: FFmpeg libavformat demuxers (wavdec, aiffdec, flacdec, oggparse*, mov, matroskadec,
// asfdec, rmdec, aadec, act, dss, mmf, vocdec, wvdec, ape, tta, mpc, qoadec, amr, au) and the
// specs: WAVE (Microsoft), AIFF 1.3 (Apple), FLAC (RFC 9639), ISO 14496-12 (MP4),
// RFC 3533 / 7845 (Ogg, Opus), ISO 11172-3 (MPEG audio), ISO 14496-3 (ADTS).
constexpr Layout layouts[]{
    // .wav  "RIFF" 0, "WAVE" 8, chunk "fmt ": channels +10, rate +12, bits +22
    {.extension = ".wav",
     .checks = {{{0, "RIFF"}, {8, "WAVE"}}},
     .anchor = "fmt ",
     .sampleRate = {.read = Read::Le32, .offset = 12},
     .numChannels = {.read = Read::Le16, .offset = 10},
     .bitDepth = {.read = Read::Le16, .offset = 22}},
    // .rf64  "RF64" 0, "WAVE" 8, then like .wav
    {.extension = ".rf64",
     .checks = {{{0, "RF64"}, {8, "WAVE"}}},
     .anchor = "fmt ",
     .sampleRate = {.read = Read::Le32, .offset = 12},
     .numChannels = {.read = Read::Le16, .offset = 10},
     .bitDepth = {.read = Read::Le16, .offset = 22}},
    // .act  "RIFF" 0, "WAVE" 8, 0x84 at 256, rate 24, G.729 mono
    {.extension = ".act",
     .checks = {{{0, "RIFF"}, {8, "WAVE"}, {256, "\x84"}}},
     .sampleRate = {.read = Read::Le32, .offset = 24},
     .numChannels = constant(1),
     .bitDepth = constant(0)},
    // .cda  "RIFF" 0, "CDDA" 8: CD track shortcut, always 44100 Hz stereo 16-bit
    {.extension = ".cda",
     .checks = {{{0, "RIFF"}, {8, "CDDA"}}},
     .sampleRate = constant(44100),
     .numChannels = constant(2),
     .bitDepth = constant(16)},
    // .aiff  "FORM" 0, "AIFF" or "AIFC" 8, chunk "COMM": channels +8, bits +14, rate +16 (80-bit)
    {.extension = ".aiff",
     .checks = {{{0, "FORM"}, {8, "AIFF"}}},
     .anchor = "COMM",
     .sampleRate = {.read = Read::Extended80, .offset = 16},
     .numChannels = {.read = Read::Be16, .offset = 8},
     .bitDepth = {.read = Read::Be16, .offset = 14}},
    {.extension = ".aiff",
     .checks = {{{0, "FORM"}, {8, "AIFC"}}},
     .anchor = "COMM",
     .sampleRate = {.read = Read::Extended80, .offset = 16},
     .numChannels = {.read = Read::Be16, .offset = 8},
     .bitDepth = {.read = Read::Be16, .offset = 14}},
    // .8svx  "FORM" 0, "8SVX" 8, chunk "VHDR": rate +20, mono 8-bit
    {.extension = ".8svx",
     .checks = {{{0, "FORM"}, {8, "8SVX"}}},
     .anchor = "VHDR",
     .sampleRate = {.read = Read::Be16, .offset = 20},
     .numChannels = constant(1),
     .bitDepth = constant(8)},
    // .au  ".snd" 0, encoding 12, rate 16, channels 20
    {.extension = ".au",
     .checks = {{{0, ".snd"}}},
     .sampleRate = {.read = Read::Be32, .offset = 16},
     .numChannels = {.read = Read::Be32, .offset = 20},
     .bitDepth = {.read = Read::Be32, .offset = 12, .table = auBits}},
    // .flac  "fLaC", STREAMINFO at +8: rate 20 bits at +18, channels-1 3 bits at +20, bits-1 5 bits
    {.extension = ".flac",
     .anchor = "fLaC",
     .sampleRate = {.read = Read::Be24, .offset = 18, .shift = 4, .mask = 0xFFFFF},
     .numChannels = {.read = Read::Byte, .offset = 20, .shift = 1, .mask = 0x7, .add = 1},
     .bitDepth = {.read = Read::Be16, .offset = 20, .shift = 4, .mask = 0x1F, .add = 1}},
    // .ogg  "OggS" 0, packet "\x01vorbis": channels +11, rate +12
    {.extension = ".ogg",
     .checks = {{{0, "OggS"}}},
     .anchor = "\x01vorbis",
     .sampleRate = {.read = Read::Le32, .offset = 12},
     .numChannels = {.read = Read::Byte, .offset = 11},
     .bitDepth = constant(0)},
    // .opus  "OggS" 0, packet "OpusHead": channels +9, always 48000 Hz
    {.extension = ".opus",
     .checks = {{{0, "OggS"}}},
     .anchor = "OpusHead",
     .sampleRate = constant(48000),
     .numChannels = {.read = Read::Byte, .offset = 9},
     .bitDepth = constant(0)},
    // .oga  Ogg with Vorbis, Opus, FLAC or Speex (rate +36, channels +48)
    {.extension = ".oga",
     .checks = {{{0, "OggS"}}},
     .anchor = "\x01vorbis",
     .sampleRate = {.read = Read::Le32, .offset = 12},
     .numChannels = {.read = Read::Byte, .offset = 11},
     .bitDepth = constant(0)},
    {.extension = ".oga",
     .checks = {{{0, "OggS"}}},
     .anchor = "OpusHead",
     .sampleRate = constant(48000),
     .numChannels = {.read = Read::Byte, .offset = 9},
     .bitDepth = constant(0)},
    {.extension = ".oga",
     .checks = {{{0, "OggS"}}},
     .anchor = "fLaC",
     .sampleRate = {.read = Read::Be24, .offset = 18, .shift = 4, .mask = 0xFFFFF},
     .numChannels = {.read = Read::Byte, .offset = 20, .shift = 1, .mask = 0x7, .add = 1},
     .bitDepth = {.read = Read::Be16, .offset = 20, .shift = 4, .mask = 0x1F, .add = 1}},
    {.extension = ".oga",
     .checks = {{{0, "OggS"}}},
     .anchor = "Speex   ",
     .sampleRate = {.read = Read::Le32, .offset = 36},
     .numChannels = {.read = Read::Le32, .offset = 48},
     .bitDepth = constant(0)},
    // .mogg  version 10 (not encrypted) at 0, then Ogg Vorbis
    {.extension = ".mogg",
     .checks = {{{0, "\x0A\x00\x00\x00"sv}}},
     .anchor = "\x01vorbis",
     .sampleRate = {.read = Read::Le32, .offset = 12},
     .numChannels = {.read = Read::Byte, .offset = 11},
     .bitDepth = constant(0)},
    // .mp1 .mp2 .mp3  frame: FF, byte 1 = 111 + version (2 bits) + layer (2 bits) + 1 bit,
    // rate index byte 2 bits 2-3, channel mode byte 3 bits 6-7. One layout per MPEG version.
    {.extension = ".mp1",
     .anchor = "\xFF\xFE",
     .anchorMask = "\xFF\xFE",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0x3, .table = mpeg1Rates},
     .numChannels =
         {.read = Read::Byte, .offset = 3, .shift = 6, .mask = 0x3, .table = mpegChannels},
     .bitDepth = constant(0)},
    {.extension = ".mp1",
     .anchor = "\xFF\xF6",
     .anchorMask = "\xFF\xFE",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0x3, .table = mpeg2Rates},
     .numChannels =
         {.read = Read::Byte, .offset = 3, .shift = 6, .mask = 0x3, .table = mpegChannels},
     .bitDepth = constant(0)},
    {.extension = ".mp2",
     .anchor = "\xFF\xFC",
     .anchorMask = "\xFF\xFE",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0x3, .table = mpeg1Rates},
     .numChannels =
         {.read = Read::Byte, .offset = 3, .shift = 6, .mask = 0x3, .table = mpegChannels},
     .bitDepth = constant(0)},
    {.extension = ".mp2",
     .anchor = "\xFF\xF4",
     .anchorMask = "\xFF\xFE",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0x3, .table = mpeg2Rates},
     .numChannels =
         {.read = Read::Byte, .offset = 3, .shift = 6, .mask = 0x3, .table = mpegChannels},
     .bitDepth = constant(0)},
    {.extension = ".mp3",
     .anchor = "\xFF\xFA",
     .anchorMask = "\xFF\xFE",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0x3, .table = mpeg1Rates},
     .numChannels =
         {.read = Read::Byte, .offset = 3, .shift = 6, .mask = 0x3, .table = mpegChannels},
     .bitDepth = constant(0)},
    {.extension = ".mp3",
     .anchor = "\xFF\xF2",
     .anchorMask = "\xFF\xFE",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0x3, .table = mpeg2Rates},
     .numChannels =
         {.read = Read::Byte, .offset = 3, .shift = 6, .mask = 0x3, .table = mpegChannels},
     .bitDepth = constant(0)},
    {.extension = ".mp3",
     .anchor = "\xFF\xE2",
     .anchorMask = "\xFF\xFE",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0x3, .table = mpeg25Rates},
     .numChannels =
         {.read = Read::Byte, .offset = 3, .shift = 6, .mask = 0x3, .table = mpegChannels},
     .bitDepth = constant(0)},
    // .aac  ADTS frame FFF, rate index byte 2 bits 2-5, channels byte 2 bit 0 + byte 3 bits 6-7
    {.extension = ".aac",
     .anchor = "\xFF\xF0",
     .anchorMask = "\xFF\xF6",
     .sampleRate = {.read = Read::Byte, .offset = 2, .shift = 2, .mask = 0xF, .table = aacRates},
     .numChannels = {.read = Read::Be16, .offset = 2, .shift = 6, .mask = 0x7},
     .bitDepth = constant(0)},
    // MP4  "ftyp" 4, box "stsd": channels +36, sample size +38, rate +44
    {.extension = ".m4a",
     .checks = {{{4, "ftyp"}}},
     .anchor = "stsd",
     .sampleRate = {.read = Read::Be16, .offset = mp4SampleRate},
     .numChannels = {.read = Read::Be16, .offset = mp4Channels},
     .bitDepth = constant(0)},
    {.extension = ".m4b",
     .checks = {{{4, "ftyp"}}},
     .anchor = "stsd",
     .sampleRate = {.read = Read::Be16, .offset = mp4SampleRate},
     .numChannels = {.read = Read::Be16, .offset = mp4Channels},
     .bitDepth = constant(0)},
    {.extension = ".m4p",
     .checks = {{{4, "ftyp"}}},
     .anchor = "stsd",
     .sampleRate = {.read = Read::Be16, .offset = mp4SampleRate},
     .numChannels = {.read = Read::Be16, .offset = mp4Channels},
     .bitDepth = constant(0)},
    {.extension = ".aax",
     .checks = {{{4, "ftyp"}}},
     .anchor = "stsd",
     .sampleRate = {.read = Read::Be16, .offset = mp4SampleRate},
     .numChannels = {.read = Read::Be16, .offset = mp4Channels},
     .bitDepth = constant(0)},
    {.extension = ".3gp",
     .checks = {{{4, "ftyp"}}},
     .anchor = "stsd",
     .sampleRate = {.read = Read::Be16, .offset = mp4SampleRate},
     .numChannels = {.read = Read::Be16, .offset = mp4Channels},
     .bitDepth = constant(0)},
    {.extension = ".alac",
     .checks = {{{4, "ftyp"}}},
     .anchor = "stsd",
     .sampleRate = {.read = Read::Be16, .offset = mp4SampleRate},
     .numChannels = {.read = Read::Be16, .offset = mp4Channels},
     .bitDepth = {.read = Read::Be16, .offset = mp4SampleSize}},
    // .webm  EBML 1A45DFA3 0, element B5 = rate (float, 8 or 4 bytes), element 9F = channels
    {.extension = ".webm",
     .checks = {{{0, "\x1A\x45\xDF\xA3"}}},
     .anchor = "\xB5\x88",
     .sampleRate = {.read = Read::FloatBe64, .offset = 2},
     .numChannels = {.read = Read::Byte, .offset = 2, .anchor = "\x9F\x81"},
     .bitDepth = constant(0)},
    {.extension = ".webm",
     .checks = {{{0, "\x1A\x45\xDF\xA3"}}},
     .anchor = "\xB5\x84",
     .sampleRate = {.read = Read::FloatBe32, .offset = 2},
     .numChannels = {.read = Read::Byte, .offset = 2, .anchor = "\x9F\x81"},
     .bitDepth = constant(0)},
    // .wma  ASF header GUID 0, audio stream GUID: channels +56, rate +58
    {.extension = ".wma",
     .checks = {{{0, "\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C"sv}}},
     .anchor = "\x40\x9E\x69\xF8\x4D\x5B\xCF\x11\xA8\xFD\x00\x80\x5F\x5C\x44\x2B"sv,
     .sampleRate = {.read = Read::Le32, .offset = 58},
     .numChannels = {.read = Read::Le16, .offset = 56},
     .bitDepth = constant(0)},
    // .ra .rm  ".ra\xFD" + version: v3 = 8000 Hz mono, v4 rate +48 channels +54,
    // v5 rate +54 channels +60. Newer .ra files are .rm files (".RMF" at 0).
    {.extension = ".ra",
     .anchor = ".ra\xFD\x00\x03"sv,
     .sampleRate = constant(8000),
     .numChannels = constant(1),
     .bitDepth = constant(0)},
    {.extension = ".ra",
     .anchor = ".ra\xFD\x00\x04"sv,
     .sampleRate = {.read = Read::Be16, .offset = 48},
     .numChannels = {.read = Read::Be16, .offset = 54},
     .bitDepth = constant(0)},
    {.extension = ".ra",
     .anchor = ".ra\xFD\x00\x05"sv,
     .sampleRate = {.read = Read::Be16, .offset = 54},
     .numChannels = {.read = Read::Be16, .offset = 60},
     .bitDepth = constant(0)},
    {.extension = ".rm",
     .checks = {{{0, ".RMF"}}},
     .anchor = ".ra\xFD\x00\x03"sv,
     .sampleRate = constant(8000),
     .numChannels = constant(1),
     .bitDepth = constant(0)},
    {.extension = ".rm",
     .checks = {{{0, ".RMF"}}},
     .anchor = ".ra\xFD\x00\x04"sv,
     .sampleRate = {.read = Read::Be16, .offset = 48},
     .numChannels = {.read = Read::Be16, .offset = 54},
     .bitDepth = constant(0)},
    {.extension = ".rm",
     .checks = {{{0, ".RMF"}}},
     .anchor = ".ra\xFD\x00\x05"sv,
     .sampleRate = {.read = Read::Be16, .offset = 54},
     .numChannels = {.read = Read::Be16, .offset = 60},
     .bitDepth = constant(0)},
};

// True when `pattern` is at `at`. With a mask, only the masked bits must match.
bool matchesAt(const vector<uint8_t>& data, size_t at, string_view pattern, string_view mask)
{
    if (at + pattern.size() > data.size()) {
        return false;
    }
    for (size_t k = 0; k < pattern.size(); ++k) {
        const uint8_t bits = mask.empty() ? 0xFF : static_cast<uint8_t>(mask[k]);
        if ((data[at + k] & bits) != (static_cast<uint8_t>(pattern[k]) & bits)) {
            return false;
        }
    }
    return true;
}

// First position of `pattern` in `data`, from `from`.
optional<size_t> find(const vector<uint8_t>& data, size_t from, string_view pattern,
                      string_view mask)
{
    for (size_t i = from; i + pattern.size() <= data.size(); ++i) {
        if (matchesAt(data, i, pattern, mask)) {
            return i;
        }
    }
    return nullopt;
}

// Reads one field at `base + field.offset`. nullopt when the file is too short.
optional<double> readField(const vector<uint8_t>& data, size_t base, const Field& field)
{
    if (field.read == Read::Constant) {
        return field.value;
    }

    static constexpr array<size_t, 10> sizes{0, 1, 2, 4, 2, 3, 4, 4, 8, 10};  // bytes per Read
    const size_t at = base + field.offset;
    if (at + sizes[static_cast<size_t>(field.read)] > data.size()) {
        return nullopt;
    }
    const auto bigEndian = [&](size_t first, size_t count) {
        uint64_t value = 0;
        for (size_t i = first; i < first + count; ++i) {
            value = (value << 8) | data[at + i];
        }
        return value;
    };
    const auto littleEndian = [&](size_t count) {
        uint64_t value = 0;
        for (size_t i = 0; i < count; ++i) {
            value |= uint64_t{data[at + i]} << (8 * i);
        }
        return value;
    };

    uint64_t raw = 0;
    switch (field.read) {
        case Read::Byte:
            raw = data[at];
            break;
        case Read::Le16:
            raw = littleEndian(2);
            break;
        case Read::Le32:
            raw = littleEndian(4);
            break;
        case Read::Be16:
            raw = bigEndian(0, 2);
            break;
        case Read::Be24:
            raw = bigEndian(0, 3);
            break;
        case Read::Be32:
            raw = bigEndian(0, 4);
            break;
        case Read::FloatBe32: {
            const auto bits = static_cast<uint32_t>(bigEndian(0, 4));
            float value = 0.0F;
            memcpy(&value, &bits, sizeof(value));
            return value;
        }
        case Read::FloatBe64: {
            const uint64_t bits = bigEndian(0, 8);
            double value = 0.0;
            memcpy(&value, &bits, sizeof(value));
            return value;
        }
        case Read::Extended80: {
            // 80-bit float: exponent (15 bits), mantissa (64 bits) = mantissa * 2^(exponent -
            // 16446)
            const auto exponent = static_cast<int>(bigEndian(0, 2) & 0x7FFF);
            return ldexp(static_cast<double>(bigEndian(2, 8)), exponent - 16446);
        }
        case Read::Constant:
            break;
    }

    raw >>= field.shift;
    if (field.mask != 0) {
        raw &= field.mask;
    }
    if (!field.table.empty()) {
        if (raw >= field.table.size()) {
            return nullopt;
        }
        raw = field.table[raw];
    }
    return static_cast<double>(raw) + field.add;
}

// Reads the file once: all of it, or its first and last megabyte when it is bigger
// (MP4 files can keep their header at the end).
vector<uint8_t> readFile(ifstream& file, uint64_t size)
{
    constexpr uint64_t part = 1 << 20;
    vector<uint8_t> data(size <= 2 * part ? size : 2 * part);
    if (size <= 2 * part) {
        file.read(reinterpret_cast<char*>(data.data()), static_cast<streamsize>(size));
    } else {
        file.read(reinterpret_cast<char*>(data.data()), part);
        file.seekg(static_cast<streamoff>(size - part));
        file.read(reinterpret_cast<char*>(data.data() + part), part);
    }
    return data;
}

optional<AudioFileErrors> RozeAudioRegistry::decode(string_view format, const string& filePath,
                                                    AudioInfo& info)
{
    info = AudioInfo{};

    bool known = false;
    for (const auto& layout : layouts) {
        known = known || layout.extension == format;
    }
    if (!known) {
        return AudioFileErrors::UnsupportedFormat;
    }

    ifstream file(filePath, ios::binary);
    error_code error;
    const uint64_t size = filesystem::file_size(filePath, error);
    if (!file || error) {
        return AudioFileErrors::CannotOpenFile;
    }
    const vector<uint8_t> data = readFile(file, size);

    // ID3v2 tag before the audio (MP3, AAC, FLAC...): "ID3", size at 6 (4 bytes of 7 bits).
    size_t start = 0;
    if (matchesAt(data, 0, "ID3", {}) && data.size() >= 10) {
        start = 10 + ((size_t{data[6]} << 21) | (size_t{data[7]} << 14) | (size_t{data[8]} << 7) |
                      data[9]);
    }

    // Find the layout of this extension that matches the file. When several match,
    // keep the one whose anchor comes first.
    const Layout* found = nullptr;
    size_t foundAnchor = 0;
    for (const auto& layout : layouts) {
        if (layout.extension != format) {
            continue;
        }
        if (layout.kind == Kind::NoHeader) {
            return AudioFileErrors::FormatHasNoHeader;
        }
        if (layout.kind == Kind::Proprietary) {
            return AudioFileErrors::ProprietaryFormat;
        }

        bool checksPass = true;
        for (const auto& check : layout.checks) {
            if (!check.bytes.empty() &&
                !matchesAt(data, start + check.offset, check.bytes, check.mask)) {
                checksPass = false;
            }
        }
        if (!checksPass) {
            continue;
        }

        const auto anchor = layout.anchor.empty()
                                ? optional<size_t>{start}
                                : find(data, start, layout.anchor, layout.anchorMask);
        if (anchor && (found == nullptr || *anchor < foundAnchor)) {
            found = &layout;
            foundAnchor = *anchor;
        }
    }
    if (found == nullptr) {
        return AudioFileErrors::HeaderDoesNotMatchExtension;
    }

    const auto readValue = [&](const Field& field) -> optional<double> {
        if (field.anchor.empty()) {
            return readField(data, foundAnchor, field);
        }
        const auto anchor = find(data, start, field.anchor, {});
        return anchor ? readField(data, *anchor, field) : nullopt;
    };
    const auto sampleRate = readValue(found->sampleRate);
    const auto numChannels = readValue(found->numChannels);
    const auto bitDepth = readValue(found->bitDepth);
    if (!sampleRate || !numChannels || !bitDepth) {
        return AudioFileErrors::CannotReadHeader;
    }

    info.sampleRate = static_cast<uint32_t>(lround(*sampleRate));
    info.numChannels = static_cast<int>(*numChannels);
    info.bitDepth = static_cast<int>(*bitDepth);

    if (info.sampleRate == 0) {
        return AudioFileErrors::InvalidSampleRate;
    }
    if (info.numChannels <= 0) {
        return AudioFileErrors::InvalidChannelCount;
    }
    if (info.bitDepth < 0 || info.bitDepth > 64) {
        return AudioFileErrors::InvalidBitDepth;
    }
    return nullopt;
}
