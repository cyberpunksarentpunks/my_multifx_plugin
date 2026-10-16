#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/ModuleFactory.h"

juce::AudioProcessorValueTreeState::ParameterLayout AetherToneProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 }, "Global Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypass", 1 }, "Bypass", false));
    return { p.begin(), p.end() };
}

AetherToneProcessor::AetherToneProcessor()
    : juce::AudioProcessor(BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createLayout())
{
    factoryPresets = getFactoryPresets();
    if (! factoryPresets.isEmpty())
        loadFactoryPreset(0);
    else
        chain.replaceWith({});
}

void AetherToneProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    chain.prepare(sampleRate, samplesPerBlock);
    dryBuffer.setSize(2, juce::jmax(samplesPerBlock, 512), false, true, true);
    mixSm.reset(sampleRate, 0.03);
    safety.prepare(sampleRate);
}

void AetherToneProcessor::releaseResources()
{
    chain.reset();
}

bool AetherToneProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    const auto& in = layouts.getMainInputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void AetherToneProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();

    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear(c, 0, n);

    float inPeak = 0.0f;
    for (int c = 0; c < chs; ++c)
        inPeak = juce::jmax(inPeak, buffer.getMagnitude(c, 0, n));
    const float prevIn = inputPeak.load(std::memory_order_relaxed);
    inputPeak.store(inPeak > prevIn ? inPeak : prevIn * 0.92f, std::memory_order_relaxed);

    const bool bypass = apvts.getRawParameterValue("bypass")->load() > 0.5f;
    if (bypass)
    {
        const float prevOut = outputPeak.load(std::memory_order_relaxed);
        outputPeak.store(inPeak > prevOut ? inPeak : prevOut * 0.92f, std::memory_order_relaxed);
        return;
    }

    if (dryBuffer.getNumSamples() >= n && dryBuffer.getNumChannels() >= chs)
    {
        for (int c = 0; c < chs; ++c)
            dryBuffer.copyFrom(c, 0, buffer, c, 0, n);
    }

    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                bpm = *b;

    chain.process(buffer, bpm);

    mixSm.setTargetValue(apvts.getRawParameterValue("mix")->load());
    for (int i = 0; i < n; ++i)
    {
        const float mix = mixSm.getNextValue();
        for (int c = 0; c < chs; ++c)
        {
            const float dry = (dryBuffer.getNumSamples() >= n && dryBuffer.getNumChannels() > c)
                                  ? dryBuffer.getSample(c, i)
                                  : 0.0f;
            buffer.setSample(c, i, dry * (1.0f - mix) + buffer.getSample(c, i) * mix);
        }
    }

    safety.process(buffer, 0.99f);

    float outPeak = 0.0f;
    for (int c = 0; c < chs; ++c)
        outPeak = juce::jmax(outPeak, buffer.getMagnitude(c, 0, n));
    const float prevOut = outputPeak.load(std::memory_order_relaxed);
    outputPeak.store(outPeak > prevOut ? outPeak : prevOut * 0.92f, std::memory_order_relaxed);
}

void AetherToneProcessor::setCurrentProgram(int index)
{
    if (juce::isPositiveAndBelow(index, factoryPresets.size()))
        loadFactoryPreset(index);
}

const juce::String AetherToneProcessor::getProgramName(int index)
{
    if (juce::isPositiveAndBelow(index, factoryPresets.size()))
        return factoryPresets.getReference(index).name;
    return {};
}

void AetherToneProcessor::loadFactoryPreset(int index)
{
    if (! juce::isPositiveAndBelow(index, factoryPresets.size()))
        return;
    currentProgram = index;
    const auto& p = factoryPresets.getReference(index);
    chain.loadState(p.chain);
    currentPresetName = p.name;
    sendChangeMessage();
}

void AetherToneProcessor::loadChainTree(const juce::ValueTree& chainTree, const juce::String& name)
{
    chain.loadState(chainTree);
    currentPresetName = name;
    sendChangeMessage();
}

bool AetherToneProcessor::saveUserPreset(const juce::String& name)
{
    auto file = userPresetDirectory().getChildFile(name + ".aethertone");
    auto xml = chain.saveState().createXml();
    if (xml == nullptr)
        return false;
    xml->setAttribute("presetName", name);
    return xml->writeTo(file);
}

juce::StringArray AetherToneProcessor::listUserPresets() const
{
    juce::StringArray names;
    for (auto& f : userPresetDirectory().findChildFiles(juce::File::findFiles, false, "*.aethertone"))
        names.add(f.getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

bool AetherToneProcessor::loadUserPreset(const juce::String& name)
{
    return loadPresetFile(userPresetDirectory().getChildFile(name + ".aethertone"));
}

bool AetherToneProcessor::loadPresetFile(const juce::File& file)
{
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
        return false;
    auto tree = juce::ValueTree::fromXml(*xml);
    if (! tree.hasType("CHAIN"))
        return false;
    loadChainTree(tree, xml->getStringAttribute("presetName", file.getFileNameWithoutExtension()));
    return true;
}

bool AetherToneProcessor::exportCurrentPreset(const juce::File& file) const
{
    auto xml = chain.saveState().createXml();
    if (xml == nullptr)
        return false;
    xml->setAttribute("presetName", currentPresetName);
    return xml->writeTo(file);
}

void AetherToneProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree root("AETHER");
    root.setProperty("presetName", currentPresetName, nullptr);
    root.setProperty("program", currentProgram, nullptr);
    root.appendChild(apvts.copyState(), nullptr);
    root.appendChild(chain.saveState(), nullptr);
    if (auto xml = root.createXml())
        copyXmlToBinary(*xml, destData);
}

void AetherToneProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr)
        return;
    auto root = juce::ValueTree::fromXml(*xml);
    if (! root.hasType("AETHER"))
        return;
    currentPresetName = root.getProperty("presetName", "Custom").toString();
    currentProgram = (int) root.getProperty("program", 0);
    auto params = root.getChildWithName(apvts.state.getType());
    if (params.isValid())
        apvts.replaceState(params);
    auto ch = root.getChildWithName("CHAIN");
    if (ch.isValid())
        chain.loadState(ch);
    sendChangeMessage();
}

juce::AudioProcessorEditor* AetherToneProcessor::createEditor()
{
    return new AetherToneEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AetherToneProcessor();
}
