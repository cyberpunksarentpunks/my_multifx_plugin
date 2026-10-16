#include "DriveModule.h"

DriveModule::DriveModule()
{
    typeId = "drive";
    displayName = "Drive";
    accent = juce::Colour(0xffe07a5f);
    addParam({ "drive", "Drive", 0.0f, 1.0f, 0.35f, 0.01f });
    addParam({ "tone", "Tone", 0.0f, 1.0f, 0.55f, 0.01f });
    addParam({ "output", "Output", -24.0f, 12.0f, 0.0f, 0.1f, " dB" });
    addParam({ "character", "Character", 0.0f, 3.0f, 0.0f, 1.0f, {}, true, { "Warm", "Crunch", "Fuzz", "High-Gain" } });
}

void DriveModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    driveSm.reset(sampleRate, 0.03);
    toneSm.reset(sampleRate, 0.03);
    outSm.reset(sampleRate, 0.03);
    for (auto& f : toneLp)
        f.setLP(6000.0f, sampleRate);
}

void DriveModule::reset()
{
    for (auto& f : toneLp) f.reset();
    for (auto& d : dc) d.reset();
}

void DriveModule::process(juce::AudioBuffer<float>& buffer)
{
    driveSm.setTargetValue(p("drive"));
    toneSm.setTargetValue(p("tone"));
    outSm.setTargetValue(atone::dbToGain(p("output")));
    const int character = (int) std::round(p("character"));

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        const float drive = driveSm.getNextValue();
        const float tone = toneSm.getNextValue();
        const float outG = outSm.getNextValue();
        const float cutoff = 400.0f + tone * 9000.0f;

        for (int c = 0; c < chs; ++c)
        {
            toneLp[c & 1].setLP(cutoff, currentSampleRate);
            float x = buffer.getSample(c, i);
            float y = x;

            switch (character)
            {
                case 1: // Crunch
                {
                    x *= (1.0f + drive * 18.0f);
                    y = atone::cubicDrive(x * 0.35f) * 1.4f;
                    break;
                }
                case 2: // Fuzz
                {
                    x *= (1.0f + drive * 40.0f);
                    y = atone::hardClip(atone::softClip(x * 0.2f) * 3.0f, 0.9f);
                    y = y - 0.15f * y * y * y;
                    break;
                }
                case 3: // High-gain
                {
                    x *= (1.0f + drive * 28.0f);
                    y = atone::softClip(x * 0.22f);
                    y = atone::softClip(y * 2.4f);
                    break;
                }
                default: // Warm
                {
                    x *= (1.0f + drive * 8.0f);
                    y = atone::softClip(x);
                    break;
                }
            }

            y = toneLp[c & 1].process(y);
            if (character == 3)
                y = y * 0.85f + (y - toneLp[c & 1].z) * 0.35f; // extra bite
            y = dc[c & 1].process(y) * outG;
            buffer.setSample(c, i, y);
        }
    }
}
