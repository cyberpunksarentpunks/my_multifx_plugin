#pragma once
#include <JuceHeader.h>

struct FactoryPreset
{
    juce::String name;
    juce::ValueTree chain;
};

juce::Array<FactoryPreset> getFactoryPresets();
juce::File userPresetDirectory();
