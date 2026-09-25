#pragma once
#include <string_view>

using namespace std;

enum class AudioFileErrors {
    AudioFileNotLoaded,
};

string_view errorMessage(AudioFileErrors error);
