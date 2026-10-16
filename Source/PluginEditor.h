#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/AetherLookAndFeel.h"
#include "ui/ChainStrip.h"
#include "ui/ModulePanel.h"

class AetherToneEditor : public juce::AudioProcessorEditor,
                         public juce::ChangeListener,
                         public juce::Timer
{
public:
    explicit AetherToneEditor(AetherToneProcessor&);
    ~AetherToneEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void timerCallback() override;

private:
    void refreshPresetBox();
    void paintMeter(juce::Graphics& g, juce::Rectangle<float> area, float peak, const juce::String& name);

    AetherToneProcessor& processor;
    AetherLookAndFeel lnf;
    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, saveBtn { "Save" };
    juce::TextButton exportBtn { "Export" }, importBtn { "Import" };
    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach;
    juce::ToggleButton bypass { "Bypass" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;
    ChainStrip chainStrip;
    ModulePanel modulePanel;
    juce::Label mixLabel, titleLabel, subtitleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AetherToneEditor)
};
