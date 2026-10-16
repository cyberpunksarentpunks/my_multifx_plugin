#pragma once
#include "../FxModule.h"

struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1 = 0, z2 = 0;

    void setPeak(double sr, float f, float q, float gainLin)
    {
        const float w = juce::MathConstants<float>::twoPi * (f / (float) sr);
        const float cw = std::cos(w), sw = std::sin(w);
        const float A = std::sqrt(gainLin);
        const float alpha = sw / (2.0f * juce::jmax(0.05f, q));
        const float a0 = 1.0f + alpha / A;
        b0 = (1.0f + alpha * A) / a0;
        b1 = (-2.0f * cw) / a0;
        b2 = (1.0f - alpha * A) / a0;
        a1 = (-2.0f * cw) / a0;
        a2 = (1.0f - alpha / A) / a0;
    }

    void setLowShelf(double sr, float f, float q, float gainLin)
    {
        const float A = std::sqrt(gainLin);
        const float w = juce::MathConstants<float>::twoPi * (f / (float) sr);
        const float cw = std::cos(w), sw = std::sin(w);
        const float beta = std::sqrt(A) / juce::jmax(0.05f, q);
        const float a0 = (A + 1.0f) + (A - 1.0f) * cw + beta * sw;
        b0 = A * ((A + 1.0f) - (A - 1.0f) * cw + beta * sw) / a0;
        b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cw) / a0;
        b2 = A * ((A + 1.0f) - (A - 1.0f) * cw - beta * sw) / a0;
        a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cw) / a0;
        a2 = ((A + 1.0f) + (A - 1.0f) * cw - beta * sw) / a0;
    }

    void setHighShelf(double sr, float f, float q, float gainLin)
    {
        const float A = std::sqrt(gainLin);
        const float w = juce::MathConstants<float>::twoPi * (f / (float) sr);
        const float cw = std::cos(w), sw = std::sin(w);
        const float beta = std::sqrt(A) / juce::jmax(0.05f, q);
        const float a0 = (A + 1.0f) - (A - 1.0f) * cw + beta * sw;
        b0 = A * ((A + 1.0f) + (A - 1.0f) * cw + beta * sw) / a0;
        b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cw) / a0;
        b2 = A * ((A + 1.0f) + (A - 1.0f) * cw - beta * sw) / a0;
        a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cw) / a0;
        a2 = ((A + 1.0f) - (A - 1.0f) * cw - beta * sw) / a0;
    }

    float process(float x) noexcept
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() noexcept { z1 = z2 = 0; }
};

class EqModule : public FxModule
{
public:
    EqModule();
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    Biquad low[2], mid[2], hm[2], hi[2];
};
