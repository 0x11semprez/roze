#include "interface/RozeAudioRegistryErrors.hpp"

#include <string_view>

string_view errorMessage(RozeAudioRegistryErrors error)
{
    switch (error) {
        case RozeAudioRegistryErrors::FormatIsNotSupported:
            return "roze loves you, This format is not supported by roze";
        default:
            return "roze don't understand what you trying to create";
    }
}
