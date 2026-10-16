#pragma once
#include "../FxModule.h"

class InputModule : public FxModule
{
public:
    InputModule();
    bool isFixedEndpoint() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    atone::PeakMeter meter;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> satSm;
};
