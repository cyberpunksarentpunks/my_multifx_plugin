#include "ReverbModule.h"

static void hadamard8(float* x) noexcept
{
    // In-place Hadamard, scaled by 1/sqrt(8) later
    auto step = [](float& a, float& b)
    {
        const float s = a + b;
        const float d = a - b;
        a = s;
        b = d;
    };
    step(x[0], x[1]); step(x[2], x[3]); step(x[4], x[5]); step(x[6], x[7]);
    step(x[0], x[2]); step(x[1], x[3]); step(x[4], x[6]); step(x[5], x[7]);
    step(x[0], x[4]); step(x[1], x[5]); step(x[2], x[6]); step(x[3], x[7]);
}

ReverbModule::ReverbModule()
{
    typeId = "reverb";
    displayName = "Reverb";
    accent = juce::Colour(0xff9b7dff);
    addParam({ "decay", "Decay", 0.05f, 0.99f, 0.72f, 0.001f });
    addParam({ "predelay", "Pre-Delay", 0.0f, 200.0f, 24.0f, 0.1f, " ms" });
    addParam({ "damp", "Damping", 0.0f, 1.0f, 0.35f, 0.01f });
    addParam({ "size", "Size", 0.2f, 2.0f, 1.05f, 0.01f });
    addParam({ "width", "Width", 0.0f, 2.0f, 1.1f, 0.01f });
    addParam({ "mix", "Mix", 0.0f, 1.0f, 0.35f, 0.01f });
    addParam({ "shimmer", "Shimmer", 0.0f, 1.0f, 0.0f, 0.01f });
    addParam({ "freeze", "Freeze", 0.0f, 1.0f, 0.0f, 1.0f, {}, true, { "Off", "On" } });
}

void ReverbModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    for (int i = 0; i < kLines; ++i)
        lines[i].prepare(sampleRate, 400.0f);
    predelayL.prepare(sampleRate, 220.0f);
    predelayR.prepare(sampleRate, 220.0f);
    shimmer.prepare(sampleRate);
    inputHp.setHP(120.0f, sampleRate);
    inputHp.highpass = true;
}

void ReverbModule::reset()
{
    for (int i = 0; i < kLines; ++i)
    {
        lines[i].reset();
        damp[i].reset();
    }
    predelayL.reset();
    predelayR.reset();
    shimmer.reset();
    inputHp.reset();
    lastShimmer = 0.0f;
}

void ReverbModule::process(juce::AudioBuffer<float>& buffer)
{
    const float decay = p("decay");
    const float pre = p("predelay");
    const float dampAmt = p("damp");
    const float size = p("size");
    const float width = p("width");
    const float mix = isBypassed() ? 0.0f : p("mix");
    const float shim = p("shimmer");
    const bool freeze = p("freeze") >= 0.5f;
    const float feedback = freeze ? 0.9995f : decay;
    const float inGain = freeze ? 0.0f : 1.0f;
    const float dampHz = 2000.0f + (1.0f - dampAmt) * 12000.0f;

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();
    constexpr float scale = 0.35355339f; // 1/sqrt(8)

    for (int i = 0; i < n; ++i)
    {
        float inL = buffer.getSample(0, i);
        float inR = chs > 1 ? buffer.getSample(1, i) : inL;
        predelayL.writeSample(inL);
        predelayR.writeSample(inR);
        float srcL = predelayL.readMs(juce::jmax(0.1f, pre));
        float srcR = predelayR.readMs(juce::jmax(0.1f, pre));
        float mono = inputHp.process(0.5f * (srcL + srcR) + lastShimmer * shim * 0.45f) * inGain;

        float x[kLines];
        for (int k = 0; k < kLines; ++k)
        {
            const float ms = baseMs[k] * size * 4.0f;
            x[k] = lines[k].readMs(ms);
        }

        hadamard8(x);

        for (int k = 0; k < kLines; ++k)
        {
            x[k] *= scale;
            damp[k].setLP(dampHz, currentSampleRate);
            x[k] = damp[k].process(x[k]);
            const float inputTap = (k & 1) ? srcR : srcL;
            lines[k].writeSample(atone::softClip(x[k] * feedback + mono * 0.35f + inputTap * 0.15f * inGain));
        }

        float wetL = 0.0f, wetR = 0.0f;
        for (int k = 0; k < kLines; k += 2)
        {
            wetL += x[k];
            wetR += x[k + 1];
        }

        lastShimmer = shimmer.process(0.5f * (wetL + wetR), 12.0f);
        const float mid = 0.5f * (wetL + wetR);
        const float side = 0.5f * (wetL - wetR) * width;
        wetL = mid + side;
        wetR = mid - side;

        buffer.setSample(0, i, inL * (1.0f - mix) + wetL * mix);
        if (chs > 1)
            buffer.setSample(1, i, inR * (1.0f - mix) + wetR * mix);
    }
}
