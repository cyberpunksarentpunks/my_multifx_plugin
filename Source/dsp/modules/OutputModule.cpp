#include "OutputModule.h"

OutputModule::OutputModule()
{
    typeId = "output";
    displayName = "Output";
    accent = juce::Colour(0xffb8c0cc);
    addParam({ "gain", "Gain", -24.0f, 12.0f, 0.0f, 0.1f, " dB" });
    addParam({ "width", "Width", 0.0f, 2.0f, 1.0f, 0.01f });
    addParam({ "limit", "Limiter", 0.0f, 1.0f, 1.0f, 1.0f, {}, true, { "Off", "On" } });
}

void OutputModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    gainSm.reset(sampleRate, 0.02);
    widthSm.reset(sampleRate, 0.05);
    limiter.prepare(sampleRate);
    meter.value.store(0.0f);
}

void OutputModule::reset()
{
    limiter.env = 0.0f;
}

void OutputModule::process(juce::AudioBuffer<float>& buffer)
{
    gainSm.setTargetValue(atone::dbToGain(p("gain")));
    widthSm.setTargetValue(p("width"));

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();
    for (int i = 0; i < n; ++i)
    {
        const float g = gainSm.getNextValue();
        for (int c = 0; c < chs; ++c)
            buffer.setSample(c, i, buffer.getSample(c, i) * g);
    }

    atone::applyStereoWidth(buffer, widthSm.getNextValue());

    if (p("limit") >= 0.5f)
        limiter.process(buffer, 0.98f);

    meter.process(buffer);
}
