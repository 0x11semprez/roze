#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

import roze.tests;
import roze.audiofile;

using namespace std;

namespace {

// ---------------------------------------------------------------------------
// What we expect from each file. Fill one line per file.
//
//   fileName    : file inside tests/fixtures/
//   format      : extension the engine must report (".mp3" means "really mp3")
//   sampleRate  : samples per second (Hz)
//   numChannels : 1 = mono, 2 = stereo
//   bitDepth    : bits per sample, 0 = lossy codec (no bit depth)
// ---------------------------------------------------------------------------
struct ExpectedFile {
    string_view fileName;
    string_view format;
    uint32_t sampleRate;
    int numChannels;
    int bitDepth;
};

constexpr ExpectedFile expectedFiles[]{
    // fileName      format    sampleRate  numChannels  bitDepth
    {"tone.wav", ".wav", 44100, 2, 16},   {"tone24.wav", ".wav", 44100, 2, 24},
    {"tonef.wav", ".wav", 44100, 2, 32},  {"tone.rf64", ".rf64", 44100, 2, 16},
    {"tone.aiff", ".aiff", 44100, 2, 16}, {"tone.au", ".au", 44100, 2, 16},
    {"tone.8svx", ".8svx", 16000, 1, 8},  {"tone.cda", ".cda", 44100, 2, 16},
    {"tone.flac", ".flac", 44100, 2, 16}, {"tone.ogg", ".ogg", 44100, 2, 0},
    {"tone.oga", ".oga", 44100, 2, 16},   {"tone.opus", ".opus", 48000, 2, 0},
    {"tone.mogg", ".mogg", 44100, 2, 0},  {"tone.mp3", ".mp3", 44100, 2, 0},
    {"tone.mp2", ".mp2", 44100, 2, 0},    {"tone.aac", ".aac", 44100, 2, 0},
    {"tone.m4a", ".m4a", 44100, 2, 0},    {"tone.m4b", ".m4b", 44100, 2, 0},
    {"tone.alac", ".alac", 44100, 2, 16}, {"tone.3gp", ".3gp", 44100, 2, 0},
    {"tone.wv", ".wv", 44100, 2, 16},     {"tone.tta", ".tta", 44100, 2, 16},
    {"tone.voc", ".voc", 43478, 1, 8},    {"tone.mmf", ".mmf", 8000, 1, 0},
    {"tone.wma", ".wma", 44100, 2, 0},    {"tone.webm", ".webm", 48000, 2, 0},
    {"tone.rm", ".rm", 44100, 2, 0},      {"tone.ra", ".ra", 8000, 1, 0},
    {"tone.amr", ".amr", 8000, 1, 0},     {"tone.awb", ".awb", 16000, 1, 0},
    {"tone.gsm", ".gsm", 8000, 1, 0},     {"tone.qoa", ".qoa", 44100, 2, 0},
    {"tone.sln", ".sln", 8000, 1, 16},    {"tone.vox", ".vox", 8000, 1, 0},
};

// ---------------------------------------------------------------------------
// Files that must fail to load, and the error we expect.
//
//   path  : full path to the file
//   error : error load() must return
// ---------------------------------------------------------------------------
struct ExpectedError {
    string path;
    AudioFileErrors error;
};

const string sampleDir = ROZE_SAMPLE_DIR;
const string fixturesDir = ROZE_FIXTURES_DIR;

const ExpectedError expectedErrors[]{
    // path                               error
    {sampleDir + "/does_not_exist.wav", AudioFileErrors::CannotOpenFile},
    {fixturesDir + "/tone.raw", AudioFileErrors::FormatHasNoHeader},
};

// ---------------------------------------------------------------------------
// A real file copied under a wrong extension: it must NOT load.
// Example: an mp3 renamed to .wav is not a wav.
//
//   fileName  : real file inside tests/fixtures/
//   fakeName  : name of the copy, with the wrong extension
// ---------------------------------------------------------------------------
struct WrongExtension {
    string_view fileName;
    string_view fakeName;
};

constexpr WrongExtension wrongExtensions[]{
    // fileName     fakeName
    {"tone.mp3", "roze_fake.wav"},
    {"tone.wav", "roze_fake.flac"},
    {"tone.flac", "roze_fake.mp3"},
    {"tone.ogg", "roze_fake.opus"},
};

int failures = 0;

// Prints one result line and counts the failure.
void report(string_view name, bool passed)
{
    cout << (passed ? "[PASS] " : "[FAIL] ") << name << '\n';
    if (!passed) {
        ++failures;
    }
}

// Loads the file and checks there is no error.
[[nodiscard]] bool loadWithoutError(AudioFile& audio, const string& path)
{
    const optional<AudioFileErrors> error = audio.load(path);
    if (error.has_value()) {
        cerr << path << ": " << errorMessage(*error) << '\n';
    }
    return RozeTestLibrary::assertEq(error.has_value(), false);
}

// Checks every value of one file against what we expect.
[[nodiscard]] bool checkFile(const string& path, const ExpectedFile& expected)
{
    AudioFile audio(path);
    if (!loadWithoutError(audio, path)) {
        return false;
    }
    bool ok = RozeTestLibrary::assertEq(audio.isLoaded(), true);
    ok &= RozeTestLibrary::assertEq(audio.getAudioFileFormat(), string(expected.format));
    ok &= RozeTestLibrary::assertEq(audio.getSampleRate(), expected.sampleRate);
    ok &= RozeTestLibrary::assertEq(audio.getNumChannels(), expected.numChannels);
    ok &= RozeTestLibrary::assertEq(audio.getBitDepth(), expected.bitDepth);
    ok &= RozeTestLibrary::assertEq(audio.isMono(), expected.numChannels == 1);
    ok &= RozeTestLibrary::assertEq(audio.isStereo(), expected.numChannels == 2);
    return ok;
}

// Checks that load() fails with the expected error.
// The enum cannot be printed, so we compare the error messages.
[[nodiscard]] bool checkError(const ExpectedError& expected)
{
    AudioFile audio(expected.path);
    const optional<AudioFileErrors> error = audio.load(expected.path);
    if (!RozeTestLibrary::assertEq(error.has_value(), true)) {
        return false;
    }
    bool ok = RozeTestLibrary::assertEq(errorMessage(*error), errorMessage(expected.error));
    ok &= RozeTestLibrary::assertEq(audio.isLoaded(), false);
    return ok;
}

// Copies a fixture under a wrong extension and checks load() fails.
[[nodiscard]] bool checkWrongExtension(const WrongExtension& wrong)
{
    const filesystem::path source = fixturesDir + "/" + string(wrong.fileName);
    const filesystem::path fake = filesystem::temp_directory_path() / wrong.fakeName;
    filesystem::copy_file(source, fake, filesystem::copy_options::overwrite_existing);

    AudioFile audio(fake.string());
    const optional<AudioFileErrors> error = audio.load(fake.string());
    filesystem::remove(fake);

    bool ok = RozeTestLibrary::assertEq(error.has_value(), true);
    ok &= RozeTestLibrary::assertEq(audio.isLoaded(), false);
    return ok;
}

}  // namespace

int main()
{
    // The real sample shipped with the project.
    const ExpectedFile sample{"f_major.wav", ".wav", 48000, 2, 16};
    report("sample f_major.wav", checkFile(sampleDir + "/f_major.wav", sample));

    // Every fixture: right format, right values.
    for (const ExpectedFile& expected : expectedFiles) {
        const string path = fixturesDir + "/" + string(expected.fileName);
        report(expected.fileName, checkFile(path, expected));
    }

    // Files that must fail with a precise error.
    for (const ExpectedError& expected : expectedErrors) {
        report("error: " + expected.path, checkError(expected));
    }

    // Real files with a wrong extension must not load.
    for (const WrongExtension& wrong : wrongExtensions) {
        const string name = string(wrong.fileName) + " renamed " + string(wrong.fakeName);
        report(name, checkWrongExtension(wrong));
    }

    cout << '\n'
         << (failures == 0 ? "all tests passed" : "some tests failed") << " (" << failures
         << " failed)\n";
    return failures == 0 ? 0 : 1;
}
