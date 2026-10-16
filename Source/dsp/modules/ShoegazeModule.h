#pragma once
#include "../FxModule.h"

class ShoegazeModule : public FxModule
{
public:
    ShoegazeModule();
    bool processWhenBypassed() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    atone::HermiteDelay delayL, delayR;
    atone::OnePole lowpass;
    atone::OnePole highpass;
    atone::PitchShifter shimmer;
    float lfoPhase = 0.0f;
};
