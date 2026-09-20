#include "RozeAudioRegistry.hpp"

[[nodiscard]] string_view isSuppored(const string_view& format)
{
    for (const auto& f : RozeAudioRegistry::allFormats) {
        if (format != f) [[unlikely]] {
            return errorMessage(RozeAudioRegistryErrors::FormatIsNotSupported);
        }

        return "roze support your format.";
    }
}
