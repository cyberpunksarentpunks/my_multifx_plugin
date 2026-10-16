#pragma once

#include "FxModule.h"
#include <memory>

std::unique_ptr<FxModule> createModule(const juce::String& typeId);
juce::StringArray allModuleTypeIds();
juce::String displayNameForType(const juce::String& typeId);
juce::Colour accentForType(const juce::String& typeId);
bool isEndpointType(const juce::String& typeId);
