#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
namespace RozeColours {
// Background matches the logo PNG so the logo blends in with no visible box.
const juce::Colour background{0xff1c1c1c};
const juce::Colour panel{0xff242424};
const juce::Colour outline{0xff333333};
const juce::Colour pink{0xfffbbacb};
const juce::Colour text{0xffe6e6e6};
}  // namespace RozeColours

//==============================================================================
class RozeLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
    // Set on a slider's properties to draw its arc from the centre (e.g. pitch).
    static constexpr const char* bipolarProperty = "bipolar";

    RozeLookAndFeel();

    // Sora with an explicit typeface. Plain juce::Font objects resolve through the
    // global default LookAndFeel, so they would not pick up Sora on their own.
    juce::Font getSoraFont(float height, bool bold = false) const;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

    private:
    juce::Typeface::Ptr regular;
    juce::Typeface::Ptr semiBold;
};
