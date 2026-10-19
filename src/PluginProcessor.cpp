#include "interface/PluginProcessor.hpp"

#include <BinaryData.h>

#include <memory>

#include "BinaryData.h"
#include "interface/PluginEditor.hpp"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"

import roze.audiofile;

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      parameters(*this, nullptr, "roze", createParameterLayout())
{
    attackParam = parameters.getRawParameterValue("attack");
    releaseParam = parameters.getRawParameterValue("release");

    formatManager.registerBasicFormats();

    for (int i = 0; i < numVoices; ++i) {
        synth.addVoice(new juce::SamplerVoice());
    }

    auto inputStream = std::make_unique<juce::MemoryInputStream>(
        BinaryData::f_major_wav, BinaryData::f_major_wavSize, false);

    if (const auto reader = std::unique_ptr<juce::AudioFormatReader>(
            formatManager.createReaderFor(std::move(inputStream)))) {
        // The sample is an F major chord whose root is F2 (MIDI note 41).
        setSound("f_major", *reader, 41);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout()
{
    // Times in seconds, skewed so the short times get most of the knob travel.
    const juce::NormalisableRange<float> timeRange(0.0f, 5.0f, 0.001f, 0.3f);
    const auto timeText = [](float seconds, int) {
        return seconds < 1.0f ? juce::String(juce::roundToInt(seconds * 1000.0f)) + " ms"
                              : juce::String(seconds, 2) + " s";
    };
    const auto attributes = juce::AudioParameterFloatAttributes().withStringFromValueFunction(timeText);

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"attack", 1}, "Attack",
                                                           timeRange, 0.0f, attributes));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"release", 1},
                                                           "Release", timeRange, 0.1f, attributes));
    return layout;
}

void AudioPluginAudioProcessor::setSound(const juce::String& name, juce::AudioFormatReader& reader,
                                         int originalMidiNote)
{
    // Every key of the keyboard plays the sample, pitched from the root.
    juce::BigInteger midiNotes;
    midiNotes.setRange(0, 128, true);

    const double attack = attackParam->load();
    const double release = releaseParam->load();
    const double sampleLength = 10.0;

    sampleName = name;
    synth.clearSounds();
    synth.addSound(new juce::SamplerSound(name, reader, midiNotes, originalMidiNote, attack,
                                          release, sampleLength));
}

juce::String AudioPluginAudioProcessor::loadSample(const juce::File& file, int originalMidiNote)
{
    // roze reads the header first, so a bad file gets a roze error message.
    const auto path = file.getFullPathName().toStdString();
    AudioFile audioFile{path};
    if (const auto error = audioFile.load(path)) {
        return juce::String(std::string(errorMessage(*error)));
    }

    const auto reader =
        std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
    if (reader == nullptr) {
        return juce::String(std::string(errorMessage(AudioFileErrors::UnsupportedFormat)));
    }

    setSound(file.getFileNameWithoutExtension(), *reader, originalMidiNote);
    return {};
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;  // NB: some hosts don't cope very well if you tell them there are 0
               // programs, so this should be at least 1, even if you're not really
               // implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        juce::ignoreUnused(channelData);
        // ..do something to the data...
    }

    updateEnvelope();

    // Merge notes played on the editor keyboard (mouse / QWERTY) into the host MIDI.
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

void AudioPluginAudioProcessor::updateEnvelope()
{
    // A voice reads the envelope when its note starts, so new values apply to the next notes.
    const juce::ADSR::Parameters envelope{attackParam->load(), 0.0f, 1.0f, releaseParam->load()};
    synth.setEnvelope(envelope);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto xml = parameters.copyState().createXml()) {
        copyXmlToBinary(*xml, destData);
    }
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes)) {
        if (xml->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
