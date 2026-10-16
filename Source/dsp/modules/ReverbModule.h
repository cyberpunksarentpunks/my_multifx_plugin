#pragma once
#include "../FxModule.h"

class ReverbModule : public FxModule
{
public:
    ReverbModule();
    bool processWhenBypassed() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    static constexpr int kLines = 8;
    atone::HermiteDelay lines[kLines];
    atone::OnePole damp[kLines];
    atone::HermiteDelay predelayL, predelayR;
    atone::PitchShifter shimmer;
    atone::OnePole inputHp;
    float lastShimmer = 0.0f;
    const float baseMs[kLines] { 11.73f, 19.31f, 23.71f, 29.17f, 37.13f, 41.27f, 47.87f, 53.41f };
};
