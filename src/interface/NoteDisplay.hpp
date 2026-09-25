#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

//==============================================================================
// Shows the names of the notes being played. Each name appears at once when the
// key goes down, then fades out after release.
class NoteDisplay final : public juce::Component, private juce::Timer {
    public:
    NoteDisplay(juce::MidiKeyboardState&, juce::Font);
    ~NoteDisplay() override;

    void paint(juce::Graphics&) override;

    private:
    struct NoteGlyph {
        int note = 0;
        bool held = true;
        float alpha = 1.0f;         // fades to 0 after release
        juce::Path path;            // glyph outline at referenceHeight, origin top-left
        float width = 0.0f;
    };

    void timerCallback() override;
    NoteGlyph makeGlyph(int note) const;

    static constexpr float referenceHeight = 100.0f;

    juce::MidiKeyboardState& keyboardState;
    juce::Font font;
    std::vector<NoteGlyph> glyphs;
    double lastTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteDisplay)
};
