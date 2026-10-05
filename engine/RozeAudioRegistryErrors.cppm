module;

#include <string_view>

export module roze.registry.errors;

export enum class RozeAudioRegistryErrors {
    FormatIsNotSupported,
};

export std::string_view errorMessage(RozeAudioRegistryErrors error)
{
    switch (error) {
        case RozeAudioRegistryErrors::FormatIsNotSupported:
            return "roze loves you, This format is not supported by roze";
        default:
            return "roze don't understand what you trying to create";
    }
}
