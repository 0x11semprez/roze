#pragma once
#include <array>
#include <string_view>

#include "errors/RozeAudioRegistryErrors.hpp"

using namespace std;

class RozeAudioRegistry {
    public:
    static inline constexpr array<string_view, 47> allFormats{
        ".3gp",  ".aa",     ".aac", ".aax",  ".act",  ".aiff",  ".alac", ".amr", ".ape", ".au",
        ".awb",  ".dss",    ".dvf", ".flac", ".gsm",  ".iklax", ".ivs",  ".m4a", ".m4b", ".m4p",
        ".mmf",  ".movpkg", ".mp1", ".mp2",  ".mp3",  ".mpc",   ".msv",  ".nmf", ".ogg", ".oga",
        ".mogg", ".opus",   ".qoa", ".ra",   ".rm",   ".raw",   ".rf64", ".sln", ".tta", ".voc",
        ".vox",  ".wav",    ".wma", ".wv",   ".webm", ".8svx",  ".cda"};

    [[nodiscard]] static inline int allFormatsSupportbyRoze() noexcept;
    [[nodiscard]] string_view isSuppored(const string_view& format);
};
