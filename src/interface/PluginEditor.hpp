#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

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

    // Base (unscaled) editor size: header, controls panel and keyboard, nothing else.
    static constexpr int headerHeight = 56;
    static constexpr int controlsHeight = 130;
    static constexpr int keyboardHeight = 80;
    static constexpr int margin = 12;
    static constexpr int baseWidth = 820;
    static constexpr int baseHeight = headerHeight + controlsHeight + 2 * margin + keyboardHeight;

    private:
    void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& name);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    // Declared before the child components so it outlives them.
    RozeLookAndFeel lookAndFeel;

    juce::Image logo;
    juce::MidiKeyboardComponent keyboard;

    // Pitch in semitones. UI only for now, not connected to the processor yet.
    juce::Slider pitchKnob;
    juce::Label pitchLabel;

    // Sampler envelope, attached to the processor parameters.
    juce::Slider attackKnob;
    juce::Label attackLabel;
    juce::Slider releaseKnob;
    juce::Label releaseLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment attackAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment releaseAttachment;

    // Name of the loaded sample, or the roze error message after a bad drop.
    juce::Label sampleCaption;
    juce::Label sampleLabel;
    bool isDraggingFile = false;

    juce::Rectangle<int> headerArea;
    juce::Rectangle<int> controlsArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
