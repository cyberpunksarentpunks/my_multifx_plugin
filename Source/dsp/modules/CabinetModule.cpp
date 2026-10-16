#include "CabinetModule.h"

CabinetModule::CabinetModule()
{
    typeId = "cabinet";
    displayName = "Cabinet / IR";
    accent = juce::Colour(0xffc4a574);
    addParam({ "mix", "Mix", 0.0f, 1.0f, 1.0f, 0.01f });
    addParam({ "gain", "Gain", -12.0f, 12.0f, 0.0f, 0.1f, " dB" });
}

void CabinetModule::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax(samplesPerBlock, 512);
    spec.numChannels = 2;
    convolution.prepare(spec);
    dryBuffer.setSize(2, juce::jmax(samplesPerBlock, 512), false, true, true);
    prepared = true;
}

void CabinetModule::reset()
{
    convolution.reset();
}

void CabinetModule::process(juce::AudioBuffer<float>& buffer)
{
    if (! irReady.load(std::memory_order_acquire) || ! prepared)
        return;

    const float mix = p("mix");
    const float g = atone::dbToGain(p("gain"));
    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();
    if (dryBuffer.getNumSamples() < n || dryBuffer.getNumChannels() < chs)
        return;
    for (int c = 0; c < chs; ++c)
        dryBuffer.copyFrom(c, 0, buffer, c, 0, n);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    convolution.process(ctx);

    for (int c = 0; c < chs; ++c)
    {
        auto* y = buffer.getWritePointer(c);
        const auto* d = dryBuffer.getReadPointer(juce::jmin(c, dryBuffer.getNumChannels() - 1));
        for (int i = 0; i < n; ++i)
            y[i] = (d[i] * (1.0f - mix) + y[i] * mix) * g;
    }
}

bool CabinetModule::loadImpulseResponse(const juce::File& file)
{
    if (! file.existsAsFile())
        return false;

    irReady.store(false, std::memory_order_release);
    convolution.loadImpulseResponse(file,
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::yes,
                                    0,
                                    juce::dsp::Convolution::Normalise::yes);
    irPath = file.getFullPathName();
    irReady.store(true, std::memory_order_release);
    return true;
}

void CabinetModule::extraState(juce::ValueTree& t) const
{
    t.setProperty("irPath", irPath, nullptr);
}

void CabinetModule::extraLoad(const juce::ValueTree& t)
{
    irPath = t.getProperty("irPath").toString();
    if (irPath.isNotEmpty())
        loadImpulseResponse(juce::File(irPath));
}
