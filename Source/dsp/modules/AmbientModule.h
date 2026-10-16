#pragma once
#include "../FxModule.h"

class AmbientModule : public FxModule
{
public:
    AmbientModule();
    bool processWhenBypassed() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    float env = 0.0f;
    float osc1 = 0.0f, osc2 = 0.0f, lfoPh = 0.0f;
    float noiseState = 0.0f;
    float lpZ[2] {}, bpZ[2] {};
    juce::Random rng { 0xA37E401u };
    atone::HermiteDelay chorusL, chorusR;
};
