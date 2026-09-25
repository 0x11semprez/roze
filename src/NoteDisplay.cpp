#include "interface/NoteDisplay.hpp"

#include <algorithm>

#include "interface/RozeLookAndFeel.hpp"

namespace {
constexpr float fadeSeconds = 0.5f;
constexpr int allChannels = 0xffff;
constexpr float glyphGap = 0.25f;  // gap between names, relative to height
}  // namespace

//==============================================================================
NoteDisplay::NoteDisplay(juce::MidiKeyboardState& state, juce::Font displayFont)
    : keyboardState(state), font(displayFont.withHeight(referenceHeight))
{
    setInterceptsMouseClicks(false, false);
    lastTime = juce::Time::getMillisecondCounterHiRes();
    startTimerHz(60);
}

NoteDisplay::~NoteDisplay()
{
    stopTimer();
}

NoteDisplay::NoteGlyph NoteDisplay::makeGlyph(int note) const
{
    NoteGlyph glyph;
    glyph.note = note;

    juce::GlyphArrangement arrangement;
    arrangement.addLineOfText(font, juce::MidiMessage::getMidiNoteName(note, true, true, 4), 0.0f,
                              0.0f);
    arrangement.createPath(glyph.path);

    // Move the outline so its bounding box starts at the origin.
    const auto bounds = glyph.path.getBounds();
    glyph.path.applyTransform(juce::AffineTransform::translation(-bounds.getX(), -bounds.getY()));
    glyph.width = bounds.getWidth();
    return glyph;
}

void NoteDisplay::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounterHiRes();
    const auto dt = static_cast<float>((now - lastTime) / 1000.0);
    lastTime = now;

    auto changed = false;

    // The keyboard state is shared with the audio thread, so poll it here rather
    // than listening for note callbacks that may arrive on the audio thread.
    for (int note = 0; note < 128; ++note) {
        const auto isOn = keyboardState.isNoteOnForChannels(allChannels, note);
        auto it = std::find_if(glyphs.begin(), glyphs.end(),
                               [note](const NoteGlyph& g) { return g.note == note; });

        if (isOn && it == glyphs.end()) {
            glyphs.push_back(makeGlyph(note));
            changed = true;
        } else if (isOn && !it->held) {
            // Pressed again while fading out: show it fully again.
            it->held = true;
            it->alpha = 1.0f;
            changed = true;
        } else if (!isOn && it != glyphs.end() && it->held) {
            it->held = false;
        }
    }

    for (auto& glyph : glyphs) {
        if (!glyph.held) {
            glyph.alpha = juce::jmax(0.0f, glyph.alpha - dt / fadeSeconds);
            changed = true;
        }
    }

    glyphs.erase(std::remove_if(glyphs.begin(), glyphs.end(),
                                [](const NoteGlyph& g) { return !g.held && g.alpha <= 0.0f; }),
                 glyphs.end());

    // Held notes are static, so only repaint when something appears or fades.
    if (changed) repaint();
}

void NoteDisplay::paint(juce::Graphics& g)
{
    if (glyphs.empty()) return;

    auto sorted = glyphs;
    std::sort(sorted.begin(), sorted.end(),
              [](const NoteGlyph& a, const NoteGlyph& b) { return a.note < b.note; });

    const auto height = static_cast<float>(getHeight()) * 0.45f;
    const auto scale = height / referenceHeight;
    const auto y = (static_cast<float>(getHeight()) - height) / 2.0f;

    // Shrink everything if a big chord does not fit in the available width.
    auto totalWidth = 0.0f;
    for (const auto& glyph : sorted) totalWidth += (glyph.width + glyphGap * referenceHeight);
    const auto fit = juce::jmin(1.0f, static_cast<float>(getWidth()) / (totalWidth * scale));

    // Centre the row; the trailing gap after the last name is not part of it.
    const auto rowWidth = (totalWidth - glyphGap * referenceHeight) * scale * fit;
    auto x = (static_cast<float>(getWidth()) - rowWidth) / 2.0f;
    for (const auto& glyph : sorted) {
        const auto transform = juce::AffineTransform::scale(scale * fit)
                                   .translated(x, y + height * (1.0f - fit) / 2.0f);
        g.setColour(RozeColours::pink.withMultipliedAlpha(glyph.alpha));
        g.fillPath(glyph.path, transform);

        x += (glyph.width + glyphGap * referenceHeight) * scale * fit;
    }
}
