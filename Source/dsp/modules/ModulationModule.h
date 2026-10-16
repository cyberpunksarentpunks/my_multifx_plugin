#pragma once
#include "../FxModule.h"

class ModulationModule : public FxModule
{
public:
    ModulationModule();
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    atone::HermiteDelay delayL, delayR;
    float lfoPhase = 0.0f;
    float allpassZ[8] {};
    juce::SmoothedValue<float> rateSm, depthSm, mixSm, fbSm;
};
