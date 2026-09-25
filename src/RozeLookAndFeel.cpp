#include "interface/RozeLookAndFeel.hpp"

#include <BinaryData.h>

RozeLookAndFeel::RozeLookAndFeel()
    : regular(juce::Typeface::createSystemTypefaceFor(BinaryData::SoraRegular_ttf,
                                                      BinaryData::SoraRegular_ttfSize)),
      semiBold(juce::Typeface::createSystemTypefaceFor(BinaryData::SoraSemiBold_ttf,
                                                       BinaryData::SoraSemiBold_ttfSize))
{
    setColour(juce::ResizableWindow::backgroundColourId, RozeColours::background);
    setColour(juce::Label::textColourId, RozeColours::text);
    setColour(juce::Slider::textBoxTextColourId, RozeColours::text);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::backgroundColourId, RozeColours::panel);
    setColour(juce::ComboBox::outlineColourId, RozeColours::outline);
    setColour(juce::ComboBox::textColourId, RozeColours::text);
    setColour(juce::ComboBox::arrowColourId, RozeColours::pink);
    setColour(juce::PopupMenu::backgroundColourId, RozeColours::panel);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, RozeColours::pink);
    setColour(juce::PopupMenu::highlightedTextColourId, RozeColours::background);
}

juce::Font RozeLookAndFeel::getSoraFont(float height, bool bold) const
{
    return juce::FontOptions(height).withTypeface(bold ? semiBold : regular);
}

juce::Font RozeLookAndFeel::getLabelFont(juce::Label& label)
{
    const auto& font = label.getFont();
    return getSoraFont(font.getHeight(), font.isBold());
}

juce::Font RozeLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    return getSoraFont(juce::jmin(15.0f, static_cast<float>(box.getHeight()) * 0.55f));
}

juce::Font RozeLookAndFeel::getPopupMenuFont()
{
    return getSoraFont(15.0f);
}

void RozeLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float rotaryStartAngle,
                                       float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Bipolar knobs fill from the middle of the range instead of from the start.
    const auto bipolar = static_cast<bool>(slider.getProperties()[bipolarProperty]);
    const auto fillFrom = bipolar ? (rotaryStartAngle + rotaryEndAngle) / 2.0f : rotaryStartAngle;

    const auto pointOnCircle = [centre](float r, float a) {
        return centre.getPointOnCircumference(r, a);
    };

    // Ring of dots, lit in pink up to the value.
    constexpr int numDots = 25;
    const auto dotRadius = radius - 3.0f;
    const auto lo = juce::jmin(fillFrom, angle);
    const auto hi = juce::jmax(fillFrom, angle);

    for (int i = 0; i < numDots; ++i) {
        const auto a = rotaryStartAngle + static_cast<float>(i) / (numDots - 1) *
                                              (rotaryEndAngle - rotaryStartAngle);
        const auto lit = a >= lo - 0.001f && a <= hi + 0.001f;
        g.setColour(lit ? RozeColours::pink : RozeColours::outline);
        g.fillEllipse(juce::Rectangle<float>(4.0f, 4.0f).withCentre(pointOnCircle(dotRadius, a)));
    }

    // Dark cap with a short pink pointer.
    const auto capRadius = radius * 0.6f;
    g.setColour(RozeColours::panel.brighter(0.08f));
    g.fillEllipse(juce::Rectangle<float>(capRadius * 2.0f, capRadius * 2.0f).withCentre(centre));
    g.setColour(RozeColours::pink);
    g.drawLine({pointOnCircle(capRadius * 0.4f, angle), pointOnCircle(capRadius * 0.9f, angle)},
               2.0f);
}
