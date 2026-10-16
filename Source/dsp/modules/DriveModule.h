#pragma once
#include "../FxModule.h"

class DriveModule : public FxModule
{
public:
    DriveModule();
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    juce::SmoothedValue<float> driveSm, toneSm, outSm;
    atone::OnePole toneLp[2];
    atone::DCBlock dc[2];
};
