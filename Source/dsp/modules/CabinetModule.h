#pragma once
#include "../FxModule.h"

class CabinetModule : public FxModule
{
public:
    CabinetModule();
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void extraState(juce::ValueTree& t) const override;
    void extraLoad(const juce::ValueTree& t) override;

    bool loadImpulseResponse(const juce::File& file);
    juce::String irPath;
    std::atomic<bool> irReady { false };

private:
    juce::dsp::Convolution convolution { juce::dsp::Convolution::Latency { 0 } };
    juce::dsp::ProcessSpec spec {};
    juce::AudioBuffer<float> dryBuffer;
    bool prepared = false;
};
