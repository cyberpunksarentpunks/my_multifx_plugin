#include "CosmicSpaceModule.h"

static void hadamard8(float* x) noexcept
{
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

CosmicSpaceModule::CosmicSpaceModule()
{
    typeId = "cosmicspace";
    displayName = "CosmicSpace";
    accent = juce::Colour(0xff6ec6ff);
    addParam({ "decay", "Decay", 0.2f, 0.99f, 0.88f, 0.005f });
    addParam({ "predelay", "Pre-Delay", 0.0f, 120.0f, 24.0f, 0.5f, " ms" });
    addParam({ "size", "Size", 0.3f, 3.0f, 1.7f, 0.01f });
    addParam({ "diffusion", "Diffusion", 0.0f, 1.0f, 0.8f, 0.01f });
    addParam({ "width", "Width", 0.0f, 2.0f, 1.45f, 0.01f });
    addParam({ "mix", "Mix", 0.0f, 1.0f, 0.5f, 0.01f });
    addParam({ "shimmer", "Shimmer", 0.0f, 1.0f, 0.35f, 0.01f });
    addParam({ "modRate", "Mod Rate", 0.01f, 0.8f, 0.18f, 0.005f, " Hz" });
    addParam({ "depth", "Depth", 0.0f, 1.0f, 0.6f, 0.01f });
    addParam({ "cutoff", "Cutoff", 400.0f, 12000.0f, 2200.0f, 1.0f, " Hz" });
}

void CosmicSpaceModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    for (int i = 0; i < kLines; ++i)
        lines[i].prepare(sampleRate, 1200.0f);
    predelayL.prepare(sampleRate, 220.0f);
    predelayR.prepare(sampleRate, 220.0f);
    shimmer.prepare(sampleRate);
    inputHp.setHP(120.0f, sampleRate);
    lfoPhase = 0.0f;
}

void CosmicSpaceModule::reset()
{
    for (int i = 0; i < kLines; ++i)
        lines[i].reset();
    predelayL.reset();
    predelayR.reset();
    shimmer.reset();
    inputHp.reset();
    lastShimmer = 0.0f;
    lfoPhase = 0.0f;
}

void CosmicSpaceModule::process(juce::AudioBuffer<float>& buffer)
{
    const float decay = p("decay");
    const float pre = p("predelay");
    const float size = p("size");
    const float diffusion = p("diffusion");
    const float width = p("width");
    const float mix = isBypassed() ? 0.0f : p("mix");
    const float shimmerAmt = p("shimmer");
    const float modRate = p("modRate");
    const float depth = p("depth");
    const float cutoff = p("cutoff");

    constexpr float scale = 0.35355339f;
    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        float inL = buffer.getSample(0, i);
        float inR = chs > 1 ? buffer.getSample(1, i) : inL;
        predelayL.writeSample(inL);
        predelayR.writeSample(inR);

        const float srcL = predelayL.readMs(juce::jmax(0.1f, pre));
        const float srcR = predelayR.readMs(juce::jmax(0.1f, pre));
        const float monoIn = inputHp.process(0.5f * (srcL + srcR) + lastShimmer * shimmerAmt * 0.3f);

        lfoPhase += modRate / (float) currentSampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
        const float mod = std::sin(juce::MathConstants<float>::twoPi * lfoPhase) * depth;

        float x[kLines];
        for (int k = 0; k < kLines; ++k)
        {
            const float drift = 1.0f + 0.7f * mod * (0.35f + (float) k / (float) kLines);
            const float ms = baseMs[k] * size * drift * (1.0f + 0.35f * diffusion);
            x[k] = lines[k].readMs(ms);
        }

        hadamard8(x);

        for (int k = 0; k < kLines; ++k)
        {
            x[k] *= scale;
            const float dampHz = juce::jlimit(200.0f, 18000.0f,
                cutoff * (0.8f + (1.0f - diffusion) * 0.9f));
            damp[k].setLP(dampHz, currentSampleRate);
            x[k] = damp[k].process(x[k]);

            const float inputTap = (k & 1) ? srcR : srcL;
            const float feed = x[k] * (decay + diffusion * 0.08f) + monoIn * 0.25f + inputTap * 0.18f;
            lines[k].writeSample(atone::softClip(feed));
        }

        float wetL = 0.0f;
        float wetR = 0.0f;
        for (int k = 0; k < kLines; k += 2)
        {
            wetL += x[k];
            wetR += x[k + 1];
        }

        const float shimmered = shimmer.process(0.5f * (wetL + wetR), 12.0f + 12.0f * shimmerAmt);
        lastShimmer = shimmered;

        const float cloudL = wetL + shimmered * (0.2f + shimmerAmt * 0.8f);
        const float cloudR = wetR + shimmered * (0.2f + shimmerAmt * 0.8f);
        const float mid = 0.5f * (cloudL + cloudR);
        const float side = 0.5f * (cloudL - cloudR) * width;

        const float outL = mid + side;
        const float outR = mid - side;

        buffer.setSample(0, i, inL * (1.0f - mix) + outL * mix);
        if (chs > 1)
            buffer.setSample(1, i, inR * (1.0f - mix) + outR * mix);
    }
}
