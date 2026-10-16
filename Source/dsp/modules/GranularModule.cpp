#include "GranularModule.h"

GranularModule::GranularModule()
{
    typeId = "granular";
    displayName = "Granular";
    accent = juce::Colour(0xff7fe7d8);
    addParam({ "density", "Density", 0.25f, 10.0f, 2.2f, 0.05f, " Hz" });
    addParam({ "grainSize", "Grain Size", 20.0f, 260.0f, 90.0f, 1.0f, " ms" });
    addParam({ "pitch", "Pitch", -12.0f, 12.0f, 3.0f, 0.5f, " st" });
    addParam({ "spread", "Spread", 0.0f, 1.0f, 0.6f, 0.01f });
    addParam({ "mix", "Mix", 0.0f, 1.0f, 0.42f, 0.01f });
    addParam({ "feedback", "Feedback", 0.0f, 0.9f, 0.22f, 0.01f });
    addParam({ "cutoff", "Cutoff", 400.0f, 12000.0f, 2200.0f, 1.0f, " Hz" });
    addParam({ "shimmer", "Shimmer", 0.0f, 1.0f, 0.18f, 0.01f });
}

void GranularModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    const int maxSamples = (int) std::ceil(sampleRate * 3.0) + 64;
    history.assign((size_t) maxSamples, 0.0f);
    writePos = 0;
    grainPhase = 0.0f;
    grainPos = 0.0f;
    grainLength = 0.0f;
    grainRate = 1.0f;
    grainStartDelay = 0.0f;
    lastGrain = 0.0f;
    lfoPhase = 0.0f;
    lowZ = 0.0f;
    grainActive = false;
}

void GranularModule::reset()
{
    std::fill(history.begin(), history.end(), 0.0f);
    writePos = 0;
    grainPhase = 0.0f;
    grainPos = 0.0f;
    grainLength = 0.0f;
    grainRate = 1.0f;
    grainStartDelay = 0.0f;
    lastGrain = 0.0f;
    lfoPhase = 0.0f;
    lowZ = 0.0f;
    grainActive = false;
}

void GranularModule::process(juce::AudioBuffer<float>& buffer)
{
    const float density = p("density");
    const float grainMs = p("grainSize");
    const float pitch = p("pitch");
    const float spread = p("spread");
    const float mix = isBypassed() ? 0.0f : p("mix");
    const float feedback = p("feedback");
    const float cutoff = p("cutoff");
    const float shimmer = p("shimmer");
    const int n = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        const float dryL = buffer.getSample(0, i);
        const float dryR = channels > 1 ? buffer.getSample(1, i) : dryL;
        const float dryMono = 0.5f * (dryL + dryR);

        history[(size_t) writePos] = dryMono + lastGrain * feedback;

        grainPhase += density / (float) currentSampleRate;
        while (grainPhase >= 1.0f)
        {
            grainPhase -= 1.0f;
            const float jitter = (rng.nextFloat() * 2.0f - 1.0f) * spread;
            const float grainLenMs = juce::jlimit(20.0f, 260.0f, grainMs * (0.7f + 0.8f * spread));
            grainLength = grainLenMs * 0.001f * (float) currentSampleRate;
            grainPos = 0.0f;
            grainRate = std::pow(2.0f, (pitch + jitter * 12.0f) / 12.0f);
            grainStartDelay = 20.0f + std::abs(jitter) * 180.0f;
            grainActive = true;
        }

        float grainWet = 0.0f;
        if (grainActive)
        {
            const float readPos = (float) writePos - grainStartDelay - grainPos * grainRate;
            float readIndex = readPos;
            while (readIndex < 0.0f)
                readIndex += (float) history.size();

            const int idx0 = (int) readIndex % (int) history.size();
            const int idx1 = (idx0 + 1) % (int) history.size();
            const float frac = readIndex - std::floor(readIndex);
            const float sample0 = history[(size_t) idx0];
            const float sample1 = history[(size_t) idx1];
            float sampled = sample0 + (sample1 - sample0) * frac;

            const float env = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * (grainPos / juce::jmax(1.0f, grainLength)));
            sampled *= env * (0.65f + shimmer * 0.85f);
            grainWet = sampled;

            grainPos += 1.0f;
            if (grainPos >= grainLength)
                grainActive = false;
        }

        const float mod = 0.5f + 0.5f * std::sin(juce::MathConstants<float>::twoPi * lfoPhase);
        lfoPhase += (0.08f + density * 0.05f) / (float) currentSampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;

        const float filt = cutoff * (0.7f + mod * 0.55f);
        const float b = std::exp(-juce::MathConstants<float>::twoPi * filt / (float) currentSampleRate);
        lowZ = b * lowZ + (1.0f - b) * grainWet;
        const float dressed = grainWet * 0.7f + lowZ * 0.3f;
        const float wetL = dryL * (1.0f - mix) + dressed * mix;
        const float wetR = dryR * (1.0f - mix) + dressed * mix;

        buffer.setSample(0, i, wetL);
        if (channels > 1)
            buffer.setSample(1, i, wetR);

        writePos = (writePos + 1) % (int) history.size();
        lastGrain = dressed;
    }
}
