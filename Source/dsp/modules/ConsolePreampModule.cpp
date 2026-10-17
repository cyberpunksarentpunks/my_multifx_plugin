#include "ConsolePreampModule.h"

ConsolePreampModule::ConsolePreampModule()
{
    typeId = "consolepreamp";
    displayName = "Console Preamp";
    accent = juce::Colour(0xff5bc0eb);
    addParam({ "drive", "Drive", 0.0f, 1.0f, 0.32f, 0.01f });
    addParam({ "saturation", "Saturation", 0.0f, 1.0f, 0.7f, 0.01f });
    addParam({ "output", "Output", -18.0f, 12.0f, 0.0f, 0.1f, " dB" });
    addParam({ "voicing", "Voicing", 0.0f, 1.0f, 0.0f, 1.0f, {}, true, { "Classic", "Tube" } });
}

void ConsolePreampModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    driveSm.reset(sampleRate, 0.04);
    satSm.reset(sampleRate, 0.04);
    outSm.reset(sampleRate, 0.04);
    for (auto& lp : lowpass)
        lp.setLP(7000.0f, sampleRate);
    for (auto& hp : highpass)
        hp.setHP(60.0f, sampleRate);
}

void ConsolePreampModule::reset()
{
    for (auto& lp : lowpass) lp.reset();
    for (auto& hp : highpass) hp.reset();
    for (auto& d : dc) d.reset();
}

void ConsolePreampModule::process(juce::AudioBuffer<float>& buffer)
{
    driveSm.setTargetValue(p("drive"));
    satSm.setTargetValue(p("saturation"));
    outSm.setTargetValue(atone::dbToGain(p("output")));
    const int voicing = (int) std::round(p("voicing"));

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        const float drive = driveSm.getNextValue();
        const float sat = satSm.getNextValue();
        const float outGain = outSm.getNextValue();

        for (int c = 0; c < chs; ++c)
        {
            float x = buffer.getSample(c, i);
            float y = x;

            const float gain = 1.0f + drive * 12.0f;
            const float tone = 0.7f + sat * 1.8f;

            if (voicing == 1)
            {
                x *= gain;
                y = std::tanh(x * (0.8f + sat * 1.8f));
                y = std::tanh(y * (1.2f + drive * 3.0f));
                y = 0.65f * y + 0.35f * atone::softClip(x * 0.7f);
                y *= 1.15f + sat * 0.8f;
            }
            else
            {
                x *= gain;
                y = atone::softClip(x * (0.6f + sat * 1.4f));
                y = atone::softClip(y + 0.18f * y * y * y);
                y = 0.75f * y + 0.25f * std::tanh(x * 1.9f);
                y *= 0.85f + sat * 0.9f;
            }

            const float hpCut = 50.0f + drive * 120.0f;
            const float lpCut = 7000.0f + sat * 6000.0f + drive * 2500.0f;
            highpass[c & 1].setHP(hpCut, currentSampleRate);
            lowpass[c & 1].setLP(lpCut, currentSampleRate);

            y = highpass[c & 1].process(y);
            y = lowpass[c & 1].process(y);
            y = dc[c & 1].process(y) * outGain * (0.9f + tone * 0.4f);
            buffer.setSample(c, i, y);
        }
    }
}
