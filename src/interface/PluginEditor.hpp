#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "NoteDisplay.hpp"
#include "PluginProcessor.hpp"
#include "RozeLookAndFeel.hpp"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              public juce::FileDragAndDropTarget {
    public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    // Drop one audio file anywhere on the editor to load it as the sample.
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Base (unscaled) editor size.
    static constexpr int baseWidth = 820;
    static constexpr int baseHeight = 540;

    private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    // Declared before the child components so it outlives them.
    RozeLookAndFeel lookAndFeel;

    juce::Image logo;
    juce::MidiKeyboardComponent keyboard;

    // Played notes drawing themselves in the centre panel.
    NoteDisplay noteDisplay;

    // Pitch in semitones. UI only for now, not connected to the processor yet.
    juce::Slider pitchKnob;
    juce::Label pitchLabel;

    // Name of the loaded sample, or the roze error message after a bad drop.
    juce::Label sampleLabel;
    bool isDraggingFile = false;

    juce::Rectangle<int> headerArea;
    juce::Rectangle<int> centreArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
