#pragma once
#include "../FxModule.h"

class OutputModule : public FxModule
{
public:
    OutputModule();
    bool isFixedEndpoint() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    atone::PeakMeter meter;
    atone::SimpleLimiter limiter;
    juce::SmoothedValue<float> gainSm, widthSm;
};
