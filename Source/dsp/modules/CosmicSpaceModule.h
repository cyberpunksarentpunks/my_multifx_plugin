#pragma once
#include "../FxModule.h"

class CosmicSpaceModule : public FxModule
{
public:
    CosmicSpaceModule();
    bool processWhenBypassed() const noexcept override { return true; }
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    static constexpr int kLines = 8;
    atone::HermiteDelay lines[kLines];
    atone::OnePole damp[kLines];
    atone::HermiteDelay predelayL, predelayR;
    atone::OnePole inputHp;
    atone::PitchShifter shimmer;
    float lastShimmer = 0.0f;
    float lfoPhase = 0.0f;
    const float baseMs[kLines] { 18.12f, 26.44f, 34.76f, 41.93f, 57.21f, 62.87f, 78.61f, 95.42f };
};
