#pragma once
#include "../FxModule.h"

class ConsolePreampModule : public FxModule
{
public:
    ConsolePreampModule();
    bool processWhenBypassed() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    juce::SmoothedValue<float> driveSm, satSm, outSm;
    atone::OnePole lowpass[2];
    atone::OnePole highpass[2];
    atone::DCBlock dc[2];
};
