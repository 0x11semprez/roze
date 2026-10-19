#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor {
    public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Checks the file header with the roze engine, then loads it as the sampler sound.
    // Returns an empty string on success, else the roze error message.
    [[nodiscard]] juce::String loadSample(const juce::File& file, int originalMidiNote = 60);

    juce::MidiKeyboardState& getKeyboardState() noexcept
    {
        return keyboardState;
    }

    const juce::String& getSampleName() const noexcept
    {
        return sampleName;
    }

    juce::AudioProcessorValueTreeState& getParameters() noexcept
    {
        return parameters;
    }

    private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void setSound(const juce::String& name, juce::AudioFormatReader& reader, int originalMidiNote);

    // Copies the attack and release parameters into the sampler sound envelope.
    void updateEnvelope();

    // Synthesiser that can update the envelope of its sounds under its own lock,
    // so the audio thread never races with loadSample() swapping the sound.
    class SamplerSynth final : public juce::Synthesiser {
        public:
        void setEnvelope(const juce::ADSR::Parameters& envelope)
        {
            const juce::ScopedLock sl(lock);
            for (auto* sound : sounds) {
                if (auto* sampler = dynamic_cast<juce::SamplerSound*>(sound)) {
                    sampler->setEnvelopeParameters(envelope);
                }
            }
        }
    };

    static constexpr auto numVoices = 8;
    SamplerSynth synth;
    juce::AudioFormatManager formatManager;
    juce::MidiKeyboardState keyboardState;
    juce::String sampleName;

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
