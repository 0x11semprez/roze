#include "interface/PluginEditor.hpp"

#include <BinaryData.h>

#include "interface/PluginProcessor.hpp"
#include "juce_graphics/juce_graphics.h"

namespace {
namespace Colours = RozeColours;

constexpr int knobWidth = 90;
constexpr int captionHeight = 20;
}  // namespace

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      keyboard(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard),
      attackAttachment(p.getParameters(), "attack", attackKnob),
      releaseAttachment(p.getParameters(), "release", releaseKnob)
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

    setupKnob(pitchKnob, pitchLabel, "PITCH");
    pitchKnob.setRange(-24.0, 24.0, 1.0);
    pitchKnob.setValue(0.0, juce::dontSendNotification);
    pitchKnob.setDoubleClickReturnValue(true, 0.0);
    pitchKnob.setTextValueSuffix(" st");
    pitchKnob.getProperties().set(RozeLookAndFeel::bipolarProperty, true);

    // Range, value and text come from the parameters through the attachments.
    setupKnob(attackKnob, attackLabel, "ATTACK");
    setupKnob(releaseKnob, releaseLabel, "RELEASE");

    sampleCaption.setText("SAMPLE", juce::dontSendNotification);
    sampleCaption.setColour(juce::Label::textColourId, Colours::pink);
    sampleCaption.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    sampleCaption.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(sampleCaption);

    sampleLabel.setText(p.getSampleName(), juce::dontSendNotification);
    sampleLabel.setJustificationType(juce::Justification::centredLeft);
    sampleLabel.setColour(juce::Label::textColourId, Colours::text);
    sampleLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(sampleLabel);

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

void AudioPluginAudioProcessorEditor::setupKnob(juce::Slider& knob, juce::Label& label,
                                                const juce::String& name)
{
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    knob.setWantsKeyboardFocus(false);  // keep QWERTY notes going to the keyboard
    addAndMakeVisible(knob);

    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, Colours::pink);
    label.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    addAndMakeVisible(label);
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

    // Controls panel, also the drop zone highlighted while a file hovers.
    const auto panel = controlsArea.toFloat();
    g.setColour(isDraggingFile ? Colours::panel.overlaidWith(Colours::pink.withAlpha(0.08f))
                               : Colours::panel);
    g.fillRoundedRectangle(panel, 6.0f);
    g.setColour(isDraggingFile ? Colours::pink : Colours::outline);
    g.drawRoundedRectangle(panel.reduced(0.5f), 6.0f, isDraggingFile ? 2.0f : 1.0f);
}

//==============================================================================
bool AudioPluginAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Any single file: loadSample tells the user why roze refuses it.
    return files.size() == 1;
}

void AudioPluginAudioProcessorEditor::fileDragEnter(const juce::StringArray&, int, int)
{
    isDraggingFile = true;
    repaint();
}

void AudioPluginAudioProcessorEditor::fileDragExit(const juce::StringArray&)
{
    isDraggingFile = false;
    repaint();
}

void AudioPluginAudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int)
{
    isDraggingFile = false;
    repaint();

    const auto error = processorRef.loadSample(juce::File(files[0]));
    if (error.isEmpty()) {
        sampleLabel.setColour(juce::Label::textColourId, Colours::text);
        sampleLabel.setText(processorRef.getSampleName(), juce::dontSendNotification);
    } else {
        sampleLabel.setColour(juce::Label::textColourId, Colours::pink);
        sampleLabel.setText(error, juce::dontSendNotification);
    }

    // The drop takes focus away, give it back so QWERTY keys play notes again.
    keyboard.grabKeyboardFocus();
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

    controlsArea = bounds.reduced(scaled(margin));

    // Knobs on the left, sample name in the space left on the right.
    auto inner = controlsArea.reduced(scaled(margin), scaled(margin) / 2);
    const auto placeKnob = [&](juce::Slider& knob, juce::Label& label) {
        auto column = inner.removeFromLeft(scaled(knobWidth));
        label.setBounds(column.removeFromTop(scaled(captionHeight)));
        label.setFont(lookAndFeel.getSoraFont(13.0f * scale, true));
        knob.setBounds(column);
    };
    placeKnob(pitchKnob, pitchLabel);
    placeKnob(attackKnob, attackLabel);
    placeKnob(releaseKnob, releaseLabel);

    inner.removeFromLeft(scaled(margin * 2));
    sampleCaption.setBounds(inner.removeFromTop(scaled(captionHeight)));
    sampleCaption.setFont(lookAndFeel.getSoraFont(13.0f * scale, true));
    sampleLabel.setBounds(inner.withSizeKeepingCentre(inner.getWidth(), scaled(40)));
    sampleLabel.setFont(lookAndFeel.getSoraFont(18.0f * scale));
}
