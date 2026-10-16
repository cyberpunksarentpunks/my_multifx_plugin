#include "AmbientModule.h"

AmbientModule::AmbientModule()
{
    typeId = "ambient";
    displayName = "Ambient";
    accent = juce::Colour(0xffd980ff);
    addParam({ "volume", "Volume", -60.0f, 6.0f, -12.0f, 0.1f, " dB" });
    addParam({ "pan", "Pan", -1.0f, 1.0f, 0.0f, 0.01f });
    addParam({ "cutoff", "Filter", 200.0f, 8000.0f, 1400.0f, 1.0f, " Hz" });
    addParam({ "resonance", "Resonance", 0.1f, 8.0f, 1.8f, 0.01f });
    addParam({ "modRate", "Mod Rate", 0.02f, 2.0f, 0.12f, 0.001f, " Hz" });
    addParam({ "mix", "Mix", 0.0f, 1.0f, 0.25f, 0.01f });
    addParam({ "pitch", "Pitch", 24.0f, 84.0f, 48.0f, 1.0f, " st" });
    addParam({ "track", "Input Track", 0.0f, 1.0f, 0.45f, 0.01f });
}

void AmbientModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    chorusL.prepare(sampleRate, 30.0f);
    chorusR.prepare(sampleRate, 30.0f);
}

void AmbientModule::reset()
{
    env = 0.0f;
    osc1 = osc2 = lfoPh = 0.0f;
    lpZ[0] = lpZ[1] = bpZ[0] = bpZ[1] = 0.0f;
    chorusL.reset();
    chorusR.reset();
}

void AmbientModule::process(juce::AudioBuffer<float>& buffer)
{
    const float vol = atone::dbToGain(p("volume"));
    const float pan = p("pan");
    const float cutoff = p("cutoff");
    const float q = p("resonance");
    const float modRate = p("modRate");
    const float mix = isBypassed() ? 0.0f : p("mix");
    const float pitch = p("pitch");
    const float track = p("track");
    const float panL = std::cos((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
    const float panR = std::sin((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
    const float atk = std::exp(-1.0f / (0.08f * (float) currentSampleRate));
    const float rel = std::exp(-1.0f / (0.60f * (float) currentSampleRate));
    const float baseHz = juce::MidiMessage::getMidiNoteInHertz(pitch);

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        float inL = buffer.getSample(0, i);
        float inR = chs > 1 ? buffer.getSample(1, i) : inL;
        const float absIn = 0.5f * (std::abs(inL) + std::abs(inR));
        const float coeff = absIn > env ? atk : rel;
        env = absIn + coeff * (env - absIn);

        lfoPh += modRate / (float) currentSampleRate;
        if (lfoPh >= 1.0f) lfoPh -= 1.0f;
        const float lfo = std::sin(juce::MathConstants<float>::twoPi * lfoPh);

        const float amp = (0.15f + 0.85f * env * track + (1.0f - track) * 0.35f) * vol;
        const float f1 = baseHz * (1.0f + 0.01f * lfo);
        const float f2 = baseHz * 1.4983f * (1.0f - 0.012f * lfo); // fifth
        osc1 += f1 / (float) currentSampleRate;
        osc2 += f2 / (float) currentSampleRate;
        if (osc1 >= 1.0f) osc1 -= 1.0f;
        if (osc2 >= 1.0f) osc2 -= 1.0f;

        noiseState = 0.97f * noiseState + 0.03f * (rng.nextFloat() * 2.0f - 1.0f);
        const float osc = 0.45f * std::sin(juce::MathConstants<float>::twoPi * osc1)
                        + 0.28f * std::sin(juce::MathConstants<float>::twoPi * osc2)
                        + 0.22f * noiseState;

        // SVF lowpass
        const float fc = atone::clamp(cutoff * (1.0f + 0.25f * lfo), 80.0f, (float) currentSampleRate * 0.3f);
        const float g = std::tan(juce::MathConstants<float>::pi * fc / (float) currentSampleRate);
        const float k = 1.0f / q;
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;
        const float v3 = osc - lpZ[0] - k * bpZ[0];
        const float v1 = a1 * bpZ[0] + a2 * v3;
        const float v2 = a3 * v3 + a2 * bpZ[0] + lpZ[0];
        bpZ[0] = 2.0f * v1 - bpZ[0];
        lpZ[0] = 2.0f * v2 - lpZ[0];
        float layer = v2 * amp;

        chorusL.writeSample(layer);
        chorusR.writeSample(layer);
        const float cL = chorusL.readMs(11.0f + 3.0f * lfo);
        const float cR = chorusR.readMs(13.0f - 2.5f * lfo);
        const float wetL = (layer * 0.4f + cL * 0.6f) * panL;
        const float wetR = (layer * 0.4f + cR * 0.6f) * panR;

        buffer.setSample(0, i, inL + wetL * mix);
        if (chs > 1)
            buffer.setSample(1, i, inR + wetR * mix);
    }
}
