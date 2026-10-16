#include "DelayModule.h"

static float syncDivisionMs(int divIndex, double bpm)
{
    const float beatMs = (float) (60000.0 / juce::jmax(20.0, bpm));
    const float multipliers[] = { 0.125f, 0.25f, 0.333333f, 0.5f, 0.666667f, 0.75f, 1.0f, 1.5f, 2.0f };
    const int i = juce::jlimit(0, 8, divIndex);
    return beatMs * multipliers[i];
}

DelayModule::DelayModule()
{
    typeId = "delay";
    displayName = "Delay";
    accent = juce::Colour(0xff7dcea0);
    addParam({ "timeMs", "Time", 1.0f, 2000.0f, 420.0f, 1.0f, " ms" });
    addParam({ "sync", "Sync", 0.0f, 1.0f, 0.0f, 1.0f, {}, true, { "Free", "Tempo" } });
    addParam({ "division", "Division", 0.0f, 8.0f, 6.0f, 1.0f, {}, true,
               { "1/32", "1/16", "1/8T", "1/8", "1/4T", "Dotted 1/8", "1/4", "Dotted 1/4", "1/2" } });
    addParam({ "feedback", "Feedback", 0.0f, 0.97f, 0.35f, 0.01f });
    addParam({ "mix", "Mix", 0.0f, 1.0f, 0.35f, 0.01f });
    addParam({ "mode", "Mode", 0.0f, 2.0f, 1.0f, 1.0f, {}, true, { "Mono", "Ping-Pong", "Reverse" } });
    addParam({ "hp", "HP", 20.0f, 2000.0f, 80.0f, 1.0f, " Hz" });
    addParam({ "lp", "LP", 400.0f, 18000.0f, 6500.0f, 1.0f, " Hz" });
    addParam({ "freeze", "Freeze", 0.0f, 1.0f, 0.0f, 1.0f, {}, true, { "Off", "On" } });
}

void DelayModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    dL.prepare(sampleRate, 2200.0f);
    dR.prepare(sampleRate, 2200.0f);
    timeSm.reset(sampleRate, 0.08);
    fbSm.reset(sampleRate, 0.04);
    mixSm.reset(sampleRate, 0.03);
    hpSm.reset(sampleRate, 0.05);
    lpSm.reset(sampleRate, 0.05);
    reverseSize = (int) (sampleRate * 0.6);
    reverseBufL.assign((size_t) reverseSize, 0.0f);
    reverseBufR.assign((size_t) reverseSize, 0.0f);
    reverseWrite = 0;
}

void DelayModule::reset()
{
    dL.reset();
    dR.reset();
    lpL.reset(); lpR.reset(); hpL.reset(); hpR.reset();
    std::fill(reverseBufL.begin(), reverseBufL.end(), 0.0f);
    std::fill(reverseBufR.begin(), reverseBufR.end(), 0.0f);
}

void DelayModule::process(juce::AudioBuffer<float>& buffer)
{
    const bool sync = p("sync") >= 0.5f;
    const bool freeze = p("freeze") >= 0.5f;
    const int mode = (int) std::round(p("mode"));
    const float timeTarget = sync ? syncDivisionMs((int) std::round(p("division")), hostBpm) : p("timeMs");
    timeSm.setTargetValue(atone::clamp(timeTarget, 1.0f, 2000.0f));
    fbSm.setTargetValue(freeze ? 1.0f : p("feedback"));
    mixSm.setTargetValue(isBypassed() ? 0.0f : p("mix"));
    hpSm.setTargetValue(p("hp"));
    lpSm.setTargetValue(p("lp"));

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        const float timeMs = timeSm.getNextValue();
        float fb = atone::clamp(fbSm.getNextValue(), 0.0f, freeze ? 1.0f : 0.97f);
        const float mix = mixSm.getNextValue();
        const float hp = hpSm.getNextValue();
        const float lp = lpSm.getNextValue();
        hpL.setHP(hp, currentSampleRate); hpL.highpass = true;
        hpR.setHP(hp, currentSampleRate); hpR.highpass = true;
        lpL.setLP(lp, currentSampleRate);
        lpR.setLP(lp, currentSampleRate);

        float inL = buffer.getSample(0, i);
        float inR = chs > 1 ? buffer.getSample(1, i) : inL;

        float delayedL = dL.readMs(timeMs);
        float delayedR = dR.readMs(mode == 1 ? timeMs : timeMs * 0.98f);

        delayedL = lpL.process(hpL.process(delayedL));
        delayedR = lpR.process(hpR.process(delayedR));
        delayedL = atone::softClip(delayedL);
        delayedR = atone::softClip(delayedR);

        float wetL = delayedL;
        float wetR = delayedR;

        if (mode == 2) // reverse grain
        {
            reverseBufL[(size_t) reverseWrite] = inL;
            reverseBufR[(size_t) reverseWrite] = inR;
            const int read = (reverseWrite - 1 + reverseSize) % reverseSize;
            // play the grain backwards relative to write head
            const int grain = juce::jlimit(64, reverseSize - 1, (int) (timeMs * 0.001f * currentSampleRate));
            const int pos = (reverseWrite - (reverseWrite % grain) + (grain - 1 - (reverseWrite % grain)) + reverseSize) % reverseSize;
            wetL = reverseBufL[(size_t) pos];
            wetR = reverseBufR[(size_t) pos];
            juce::ignoreUnused(read);
            reverseWrite = (reverseWrite + 1) % reverseSize;
        }

        const float inputGain = freeze ? 0.0f : 1.0f;
        if (mode == 1) // ping-pong: swap channels in feedback
        {
            dL.writeSample(inR * inputGain + delayedR * fb);
            dR.writeSample(inL * inputGain + delayedL * fb);
        }
        else if (mode != 2)
        {
            dL.writeSample(inL * inputGain + delayedL * fb);
            dR.writeSample(inR * inputGain + delayedR * fb);
        }
        else
        {
            dL.writeSample(wetL * fb);
            dR.writeSample(wetR * fb);
            wetL = wetL + delayedL * 0.35f;
            wetR = wetR + delayedR * 0.35f;
        }

        buffer.setSample(0, i, inL * (1.0f - mix) + wetL * mix);
        if (chs > 1)
            buffer.setSample(1, i, inR * (1.0f - mix) + wetR * mix);
        else
            buffer.setSample(0, i, inL * (1.0f - mix) + 0.5f * (wetL + wetR) * mix);
    }
}
