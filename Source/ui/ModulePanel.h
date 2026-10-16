#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class ModulePanel : public juce::Component
{
public:
    explicit ModulePanel(AetherToneProcessor& p);
    void setModuleUuid(const juce::String& uuid);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    struct Knob
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::ComboBox> combo;
        juce::Identifier id;
    };

    AetherToneProcessor& processor;
    juce::String uuid;
    juce::String title;
    juce::Colour accent { 0xff5ec8e3 };
    std::vector<Knob> knobs;
    juce::TextButton loadIr { "Load IR..." };
    juce::Label irLabel;
    void rebuildControls(const juce::ValueTree& state);
};
