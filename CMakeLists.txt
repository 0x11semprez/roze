cmake_minimum_required(VERSION 3.28)
project(roze VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(roze
    COMPANY_NAME             "semprez"
    BUNDLE_ID                "com.semprez.roze"
    IS_SYNTH                 TRUE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        FALSE
    COPY_PLUGIN_AFTER_BUILD  FALSE
    PLUGIN_MANUFACTURER_CODE Smpz
    PLUGIN_CODE              Roze
    FORMATS                  VST3 Standalone
    VST3_CATEGORIES          Instrument Sampler
    PRODUCT_NAME             "roze")


target_sources(roze PRIVATE
    src/PluginEditor.cpp
    src/interface/PluginEditor.hpp
    src/PluginProcessor.cpp
    src/interface/PluginProcessor.hpp
    src/RozeLookAndFeel.cpp
    src/interface/RozeLookAndFeel.hpp
    src/NoteDisplay.cpp
    src/interface/NoteDisplay.hpp)

# Engine and tests use C++20 modules: CMake only supports them with Ninja (not Unix Makefiles).
if(NOT CMAKE_GENERATOR MATCHES "Ninja|Visual Studio")
    message(FATAL_ERROR "roze uses C++20 modules, configure with -G Ninja")
endif()
add_library(rozeEngine STATIC)
target_compile_features(rozeEngine PUBLIC cxx_std_20)
target_sources(rozeEngine PUBLIC FILE_SET CXX_MODULES FILES
    engine/AudioFileErrors.cppm
    engine/AudioFile.cppm
    engine/RozeAudioRegistry.cppm
    engine/RozeAudioRegistryErrors.cppm)

enable_testing()
add_executable(rozeTests tests/AudioFileTests.cpp)
target_sources(rozeTests PRIVATE FILE_SET CXX_MODULES FILES
    tests/RozeTestsLibraryErrors.cppm
    tests/RozeTestsLibrary.cppm)
target_compile_definitions(rozeTests PRIVATE
    ROZE_SAMPLE_DIR="${CMAKE_SOURCE_DIR}/sample"
    ROZE_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures")
target_link_libraries(rozeTests PRIVATE rozeEngine)
add_test(NAME rozeTests COMMAND rozeTests)

target_compile_definitions(roze PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

juce_add_binary_data(rozeData SOURCES sample/f_major.wav docs/logo.png
    assets/fonts/Sora-Regular.ttf assets/fonts/Sora-SemiBold.ttf)

target_link_libraries(roze
    PRIVATE
    rozeData
    rozeEngine
        juce::juce_audio_utils
        juce::juce_audio_basics
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
