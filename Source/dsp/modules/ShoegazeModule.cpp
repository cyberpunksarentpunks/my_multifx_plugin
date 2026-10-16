#include "ShoegazeModule.h"

ShoegazeModule::ShoegazeModule()
{
    typeId = "shoegaze";
    displayName = "Shoegaze";
    accent = juce::Colour(0xffb394ff);
    addParam({ "drive", "Drive", 0.0f, 2.0f, 0.7f, 0.01f });
    addParam({ "mix", "Mix", 0.0f, 1.0f, 0.52f, 0.01f });
    addParam({ "feedback", "Feedback", 0.0f, 0.96f, 0.63f, 0.01f });
    addParam({ "shimmer", "Shimmer", 0.0f, 1.0f, 0.38f, 0.01f });
    addParam({ "cutoff", "Cutoff", 300.0f, 12000.0f, 1800.0f, 1.0f, " Hz" });
    addParam({ "spread", "Spread", 0.0f, 1.0f, 0.64f, 0.01f });
    addParam({ "damp", "Damping", 0.0f, 1.0f, 0.38f, 0.01f });
    addParam({ "pan", "Pan", -1.0f, 1.0f, 0.0f, 0.01f });
}

void ShoegazeModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    delayL.prepare(sampleRate, 300.0f);
    delayR.prepare(sampleRate, 300.0f);
    shimmer.prepare(sampleRate);
    lowpass.setLP(1800.0f, sampleRate);
    highpass.setHP(120.0f, sampleRate);
    lfoPhase = 0.0f;
}

void ShoegazeModule::reset()
{
    delayL.reset();
    delayR.reset();
    shimmer.reset();
    lowpass.reset();
    highpass.reset();
    lfoPhase = 0.0f;
}

void ShoegazeModule::process(juce::AudioBuffer<float>& buffer)
{
    const float drive = p("drive");
    const float mix = isBypassed() ? 0.0f : p("mix");
    const float feedback = p("feedback");
    const float shimmerAmt = p("shimmer");
    const float cutoff = p("cutoff");
    const float spread = p("spread");
    const float damp = p("damp");
    const float pan = p("pan");
    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        float inL = buffer.getSample(0, i);
        float inR = chs > 1 ? buffer.getSample(1, i) : inL;

        const float mono = 0.5f * (inL + inR);
        const float pushed = atone::softClip(mono * (1.0f + drive * 2.4f));

        const float modDepth = 24.0f + spread * 22.0f;
        lfoPhase += (0.06f + spread * 0.12f) / (float) currentSampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
        const float mod = std::sin(juce::MathConstants<float>::twoPi * lfoPhase);

        const float delayMsL = 28.0f + (mod + 1.0f) * 0.5f * modDepth + spread * 52.0f;
        const float delayMsR = 33.0f + (1.0f - (mod + 1.0f) * 0.5f) * modDepth + spread * 60.0f;

        const float echoL = delayL.readMs(delayMsL) * (0.75f + feedback * 0.5f);
        const float echoR = delayR.readMs(delayMsR) * (0.75f + feedback * 0.5f);

        const float sparkle = shimmer.process(pushed + 0.5f * (echoL + echoR), 6.0f + shimmerAmt * 12.0f);
        const float filtered = highpass.process(lowpass.process(pushed + sparkle * (0.35f + shimmerAmt) + (echoL + echoR) * 0.15f));

        const float damped = filtered * (1.0f - damp * 0.6f) + sparkle * (0.15f + damp * 0.55f);
        delayL.writeSample(inL + damped * feedback);
        delayR.writeSample(inR + damped * feedback);

        const float panL = std::cos((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float panR = std::sin((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
        const float wetL = inL * (1.0f - mix) + (damped * panL + echoL * 0.5f) * mix;
        const float wetR = inR * (1.0f - mix) + (damped * panR + echoR * 0.5f) * mix;

        buffer.setSample(0, i, wetL);
        if (chs > 1)
            buffer.setSample(1, i, wetR);
    }
}
