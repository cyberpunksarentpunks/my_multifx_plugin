#pragma once
#include "../FxModule.h"

class DelayModule : public FxModule
{
public:
    DelayModule();
    bool processWhenBypassed() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    double hostBpm = 120.0;

private:
    atone::HermiteDelay dL, dR;
    atone::OnePole lpL, lpR, hpL, hpR;
    std::vector<float> reverseBufL, reverseBufR;
    int reverseWrite = 0, reverseSize = 0;
    juce::SmoothedValue<float> timeSm, fbSm, mixSm, hpSm, lpSm;
};
