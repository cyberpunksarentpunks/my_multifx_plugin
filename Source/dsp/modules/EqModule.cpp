#include "EqModule.h"

EqModule::EqModule()
{
    typeId = "eq";
    displayName = "EQ";
    accent = juce::Colour(0xff6ea8fe);
    addParam({ "lowFreq", "Low Freq", 40.0f, 400.0f, 120.0f, 1.0f, " Hz" });
    addParam({ "lowGain", "Low Gain", -18.0f, 18.0f, 0.0f, 0.1f, " dB" });
    addParam({ "lowQ", "Low Q", 0.3f, 4.0f, 0.7f, 0.01f });
    addParam({ "midFreq", "Mid Freq", 200.0f, 2500.0f, 700.0f, 1.0f, " Hz" });
    addParam({ "midGain", "Mid Gain", -18.0f, 18.0f, 0.0f, 0.1f, " dB" });
    addParam({ "midQ", "Mid Q", 0.3f, 8.0f, 1.0f, 0.01f });
    addParam({ "hmFreq", "High-Mid Freq", 800.0f, 6000.0f, 2500.0f, 1.0f, " Hz" });
    addParam({ "hmGain", "High-Mid Gain", -18.0f, 18.0f, 0.0f, 0.1f, " dB" });
    addParam({ "hmQ", "High-Mid Q", 0.3f, 8.0f, 1.0f, 0.01f });
    addParam({ "highFreq", "High Freq", 3000.0f, 16000.0f, 8000.0f, 1.0f, " Hz" });
    addParam({ "highGain", "High Gain", -18.0f, 18.0f, 0.0f, 0.1f, " dB" });
    addParam({ "highQ", "High Q", 0.3f, 4.0f, 0.7f, 0.01f });
}

void EqModule::prepare(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    reset();
}

void EqModule::reset()
{
    for (int c = 0; c < 2; ++c)
    {
        low[c].reset();
        mid[c].reset();
        hm[c].reset();
        hi[c].reset();
    }
}

void EqModule::process(juce::AudioBuffer<float>& buffer)
{
    for (int c = 0; c < 2; ++c)
    {
        low[c].setLowShelf(currentSampleRate, p("lowFreq"), p("lowQ"), atone::dbToGain(p("lowGain")));
        mid[c].setPeak(currentSampleRate, p("midFreq"), p("midQ"), atone::dbToGain(p("midGain")));
        hm[c].setPeak(currentSampleRate, p("hmFreq"), p("hmQ"), atone::dbToGain(p("hmGain")));
        hi[c].setHighShelf(currentSampleRate, p("highFreq"), p("highQ"), atone::dbToGain(p("highGain")));
    }

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();
    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < chs; ++c)
        {
            const int idx = c & 1;
            float x = buffer.getSample(c, i);
            x = low[idx].process(x);
            x = mid[idx].process(x);
            x = hm[idx].process(x);
            x = hi[idx].process(x);
            buffer.setSample(c, i, x);
        }
    }
}
