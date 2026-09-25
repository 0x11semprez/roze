#include "interface/PluginEditor.hpp"

#include <BinaryData.h>

#include "interface/PluginProcessor.hpp"
#include "juce_graphics/juce_graphics.h"

namespace {
namespace Colours = RozeColours;

constexpr int headerHeight = 56;
constexpr int keyboardHeight = 80;
constexpr int margin = 12;
}  // namespace

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      keyboard(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard),
      noteDisplay(p.getKeyboardState(), lookAndFeel.getSoraFont(12.0f, true))
{
    setLookAndFeel(&lookAndFeel);

    // Crop the padding around the wordmark so it can fill the header height.
    const auto fullLogo =
        juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    logo = fullLogo.getClippedImage({400, 120, 1380, 470});

    keyboard.setAvailableRange(24, 108);  // C1 .. C8
    keyboard.setKeyPressBaseOctave(5);    // QWERTY row "A" plays MIDI 60
    keyboard.setOctaveForMiddleC(4);      // label MIDI 60 as C4
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, Colours::pink);
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                       Colours::pink.withAlpha(0.35f));
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, Colours::outline);
    keyboard.setColour(juce::MidiKeyboardComponent::shadowColourId,
                       juce::Colours::transparentBlack);
    addAndMakeVisible(keyboard);
    addAndMakeVisible(noteDisplay);

    setResizable(true, true);
    setResizeLimits(baseWidth * 3 / 4, baseHeight * 3 / 4, baseWidth * 2, baseHeight * 2);
    getConstrainer()->setFixedAspectRatio(static_cast<double>(baseWidth) / baseHeight);
    setSize(baseWidth, baseHeight);

    // Give the keyboard focus once the window is on screen so the computer keys play notes.
    juce::MessageManager::callAsync(
        [safe = juce::Component::SafePointer<juce::Component>(&keyboard)] {
            if (safe != nullptr && safe->isShowing()) safe->grabKeyboardFocus();
        });
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(Colours::background);

    // Header: logo centred, separator line underneath.
    const auto logoBounds = headerArea.reduced(margin, margin / 2).toFloat();
    g.drawImage(logo, logoBounds,
                juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

    g.setColour(Colours::outline);
    g.drawHorizontalLine(headerArea.getBottom(), 0.0f, static_cast<float>(getWidth()));

    // Centre panel, the note display draws on top of it.
    g.setColour(Colours::panel);
    g.fillRoundedRectangle(centreArea.toFloat(), 6.0f);
    g.setColour(Colours::outline);
    g.drawRoundedRectangle(centreArea.toFloat(), 6.0f, 1.0f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    const auto scale = static_cast<float>(getWidth()) / baseWidth;
    const auto scaled = [scale](int v) { return juce::roundToInt(v * scale); };

    auto bounds = getLocalBounds();
    headerArea = bounds.removeFromTop(scaled(headerHeight));

    auto keyboardArea = bounds.removeFromBottom(scaled(keyboardHeight));
    keyboard.setBounds(keyboardArea);
    // 7 octaves C1..C7 plus the top C8 = 50 white keys spanning the full width.
    keyboard.setKeyWidth(static_cast<float>(keyboardArea.getWidth()) / 50.0f);
    keyboard.setLowestVisibleKey(24);

    bounds.reduce(scaled(margin), scaled(margin));

    centreArea = bounds.withTrimmedBottom(scaled(margin));
    noteDisplay.setBounds(centreArea.reduced(scaled(margin)));
}
