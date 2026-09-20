#include "RozeAudioRegistry.hpp"

[[nodiscard]] int allFormatsSupportbyRoze() noexcept
{
    return RozeAudioRegistry::allFormats.size();
}

[[nodiscard]] string_view isSupported(const string_view& format)
{
    for (const auto& f : RozeAudioRegistry::allFormats) {
        if (format != f) [[unlikely]] {
            return errorMessage(RozeAudioRegistryErrors::FormatIsNotSupported);
        }

        return "roze support your format.";
    }
}
