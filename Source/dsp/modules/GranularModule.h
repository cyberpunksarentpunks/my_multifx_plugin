#pragma once
#include "../FxModule.h"

class GranularModule : public FxModule
{
public:
    GranularModule();
    bool processWhenBypassed() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    std::vector<float> history;
    int writePos = 0;
    float grainPhase = 0.0f;
    float grainPos = 0.0f;
    float grainLength = 0.0f;
    float grainRate = 1.0f;
    float grainStartDelay = 0.0f;
    float lastGrain = 0.0f;
    float lfoPhase = 0.0f;
    float lowZ = 0.0f;
    bool grainActive = false;
    juce::Random rng { 0x6742B130u };
};
