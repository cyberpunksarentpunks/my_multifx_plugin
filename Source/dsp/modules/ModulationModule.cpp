#include "ModulationModule.h"

ModulationModule::ModulationModule()
{
    typeId = "modulation";
    displayName = "Modulation";
    accent = juce::Colour(0xff5ec8e3);
    addParam({ "type", "Type", 0.0f, 3.0f, 0.0f, 1.0f, {}, true, { "Chorus", "Vibrato", "Flanger", "Phaser" } });
    addParam({ "rate", "Rate", 0.05f, 12.0f, 0.8f, 0.01f, " Hz" });
    addParam({ "depth", "Depth", 0.0f, 1.0f, 0.45f, 0.01f });
    addParam({ "mix", "Mix", 0.0f, 1.0f, 0.5f, 0.01f });
    addParam({ "feedback", "Feedback", 0.0f, 0.85f, 0.15f, 0.01f });
}

void ModulationModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    delayL.prepare(sampleRate, 40.0f);
    delayR.prepare(sampleRate, 40.0f);
    rateSm.reset(sampleRate, 0.05);
    depthSm.reset(sampleRate, 0.05);
    mixSm.reset(sampleRate, 0.03);
    fbSm.reset(sampleRate, 0.05);
}

void ModulationModule::reset()
{
    delayL.reset();
    delayR.reset();
    lfoPhase = 0.0f;
    std::fill(std::begin(allpassZ), std::end(allpassZ), 0.0f);
}

void ModulationModule::process(juce::AudioBuffer<float>& buffer)
{
    const int type = (int) std::round(p("type"));
    rateSm.setTargetValue(p("rate"));
    depthSm.setTargetValue(p("depth"));
    mixSm.setTargetValue(p("mix"));
    fbSm.setTargetValue(p("feedback"));

    const int n = buffer.getNumSamples();
    const int chs = juce::jmax(1, buffer.getNumChannels());
    float fbStateL = 0.0f, fbStateR = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float rate = rateSm.getNextValue();
        const float depth = depthSm.getNextValue();
        const float mix = mixSm.getNextValue();
        const float fb = fbSm.getNextValue();
        lfoPhase += rate / (float) currentSampleRate;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        const float lfoL = atone::lfo(lfoPhase, 0);
        const float lfoR = atone::lfo(std::fmod(lfoPhase + 0.25f, 1.0f), 0);

        float inL = buffer.getSample(0, i);
        float inR = chs > 1 ? buffer.getSample(1, i) : inL;
        float wetL = inL, wetR = inR;

        if (type == 3) // Phaser: 4 allpass stages per channel
        {
            const float sweep = 0.15f + 0.75f * (0.5f + 0.5f * lfoL * depth);
            auto ap = [&](float x, int idx, float a)
            {
                const float y = -a * x + allpassZ[idx];
                allpassZ[idx] = y * a + x;
                return y;
            };
            float xL = inL + fbStateL * fb * 0.5f;
            float xR = inR + fbStateR * fb * 0.5f;
            for (int s = 0; s < 4; ++s)
            {
                xL = ap(xL, s, sweep);
                xR = ap(xR, s + 4, sweep * 0.97f);
            }
            fbStateL = xL;
            fbStateR = xR;
            wetL = xL;
            wetR = xR;
        }
        else
        {
            float minMs = 7.0f, rangeMs = 12.0f;
            if (type == 1) { minMs = 2.0f; rangeMs = 6.0f; }      // vibrato
            if (type == 2) { minMs = 0.8f; rangeMs = 4.5f; }      // flanger

            const float dMsL = minMs + (0.5f + 0.5f * lfoL) * rangeMs * depth;
            const float dMsR = minMs + (0.5f + 0.5f * lfoR) * rangeMs * depth * 0.92f;

            float xL = inL + fbStateL * (type == 2 ? fb : 0.0f);
            float xR = inR + fbStateR * (type == 2 ? fb : 0.0f);
            delayL.writeSample(xL);
            delayR.writeSample(xR);
            wetL = delayL.readMs(dMsL);
            wetR = delayR.readMs(dMsR);
            fbStateL = wetL;
            fbStateR = wetR;
        }

        const float dryMix = (type == 1) ? 0.0f : (1.0f - mix); // vibrato is fully wet of delayed signal
        const float wetMix = (type == 1) ? mix : mix;
        const float outL = inL * dryMix + wetL * wetMix;
        const float outR = inR * dryMix + wetR * wetMix;
        buffer.setSample(0, i, outL);
        if (chs > 1)
            buffer.setSample(1, i, outR);
    }
}
