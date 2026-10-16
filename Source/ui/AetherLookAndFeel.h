#pragma once
#include <JuceHeader.h>

class AetherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AetherLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStart, float rotaryEnd,
                          juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& bg,
                              bool highlighted, bool down) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int,
                      juce::ComboBox& box) override;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getLabelFont(juce::Label&) override;

    juce::Colour bg { 0xff0d0f14 };
    juce::Colour panel { 0xff161920 };
    juce::Colour card { 0xff1c2028 };
    juce::Colour text { 0xffe8eaef };
    juce::Colour muted { 0xff8b90a0 };
    juce::Colour accent { 0xff5ec8e3 };
};
