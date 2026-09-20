#pragma once
#include <string_view>

using namespace std;

enum class RozeAudioRegistryErrors {
    FormatIsNotSupported,
};

string_view errorMessage(RozeAudioRegistryErrors error) {}
