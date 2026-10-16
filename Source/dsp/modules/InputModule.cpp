#include "InputModule.h"

InputModule::InputModule()
{
    typeId = "input";
    displayName = "Input";
    accent = juce::Colour(0xffe8b86d);
    addParam({ "gain", "Gain", -24.0f, 24.0f, 0.0f, 0.1f, " dB" });
    addParam({ "saturate", "Saturate", 0.0f, 1.0f, 0.0f, 0.01f });
}

void InputModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    gainSm.reset(sampleRate, 0.02);
    satSm.reset(sampleRate, 0.03);
    meter.value.store(0.0f);
}

void InputModule::reset()
{
    gainSm.setCurrentAndTargetValue(atone::dbToGain(p("gain")));
    satSm.setCurrentAndTargetValue(p("saturate"));
}

void InputModule::process(juce::AudioBuffer<float>& buffer)
{
    gainSm.setTargetValue(atone::dbToGain(p("gain")));
    satSm.setTargetValue(p("saturate"));

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();
    for (int i = 0; i < n; ++i)
    {
        const float g = gainSm.getNextValue();
        const float s = satSm.getNextValue();
        for (int c = 0; c < chs; ++c)
        {
            float x = buffer.getSample(c, i) * g;
            if (s > 0.0001f)
            {
                const float driven = atone::softClip(x * (1.0f + 4.0f * s));
                x = x * (1.0f - s) + driven * s;
            }
            buffer.setSample(c, i, x);
        }
    }
    meter.process(buffer);
}
