#include "AetherLookAndFeel.h"

AetherLookAndFeel::AetherLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, bg);
    setColour(juce::Label::textColourId, text);
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff242833));
    setColour(juce::TextButton::textColourOffId, text);
    setColour(juce::TextButton::textColourOnId, bg);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff242833));
    setColour(juce::ComboBox::textColourId, text);
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff323846));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1a1d24));
    setColour(juce::PopupMenu::textColourId, text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha(0.25f));
    setColour(juce::Slider::rotarySliderFillColourId, accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a303c));
    setColour(juce::Slider::textBoxTextColourId, text);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

void AetherLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float rotaryStart, float rotaryEnd,
                                         juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(6.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto toAngle = rotaryStart + sliderPos * (rotaryEnd - rotaryStart);
    const auto lineW = juce::jmax(2.5f, radius * 0.12f);
    auto ring = juce::Rectangle<float>(radius * 2.0f, radius * 2.0f).withCentre(centre);

    g.setColour(juce::Colour(0xff2a303c));
    juce::Path bgArc;
    bgArc.addCentredArc(centre.x, centre.y, radius - lineW * 0.5f, radius - lineW * 0.5f,
                        0.0f, rotaryStart, rotaryEnd, true);
    g.strokePath(bgArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
    g.setColour(fill);
    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius - lineW * 0.5f, radius - lineW * 0.5f,
                           0.0f, rotaryStart, toAngle, true);
    g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(juce::Colour(0xff14171d));
    g.fillEllipse(ring.reduced(lineW * 1.6f));

    juce::Path pointer;
    const float inner = radius - lineW * 2.8f;
    pointer.addRoundedRectangle(-1.2f, -inner, 2.4f, inner * 0.62f, 1.0f);
    g.setColour(fill.brighter(0.25f));
    g.fillPath(pointer, juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
}

void AetherLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& bgColour,
                                             bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(0.5f);
    auto c = bgColour;
    if (b.getToggleState())
        c = accent;
    if (highlighted) c = c.brighter(0.12f);
    if (down) c = c.darker(0.08f);
    g.setColour(c);
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(juce::Colour(0xff323846).withAlpha(b.getToggleState() ? 0.0f : 1.0f));
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
}

void AetherLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float>(0, 0, (float) width, (float) height).reduced(0.5f);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
    g.setColour(accent);
    juce::Path p;
    p.addTriangle((float) width - 18.0f, (float) height * 0.4f,
                  (float) width - 10.0f, (float) height * 0.4f,
                  (float) width - 14.0f, (float) height * 0.62f);
    g.fillPath(p);
}

juce::Font AetherLookAndFeel::getTextButtonFont(juce::TextButton&, int) { return juce::Font { juce::FontOptions(13.0f) }; }
juce::Font AetherLookAndFeel::getComboBoxFont(juce::ComboBox&) { return juce::Font { juce::FontOptions(14.0f) }; }
juce::Font AetherLookAndFeel::getLabelFont(juce::Label&) { return juce::Font { juce::FontOptions(12.5f) }; }
