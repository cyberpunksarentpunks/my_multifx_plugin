#pragma once

#include <JuceHeader.h>
#include "dsp/ChainProcessor.h"
#include "presets/FactoryPresets.h"

class AetherToneProcessor : public juce::AudioProcessor,
                            public juce::ChangeBroadcaster
{
public:
    AetherToneProcessor();
    ~AetherToneProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AetherTone"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return juce::jmax(1, factoryPresets.size()); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    ChainProcessor chain;
    juce::Array<FactoryPreset> factoryPresets;
    juce::String currentPresetName { "Dreamscape Clean" };
    int currentProgram = 0;

    float getInputPeak() const { return inputPeak.load(); }
    float getOutputPeak() const { return outputPeak.load(); }

    void loadFactoryPreset(int index);
    void loadChainTree(const juce::ValueTree& chainTree, const juce::String& name);
    bool saveUserPreset(const juce::String& name);
    juce::StringArray listUserPresets() const;
    bool loadUserPreset(const juce::String& name);
    bool loadPresetFile(const juce::File& file);
    bool exportCurrentPreset(const juce::File& file) const;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    std::atomic<float> inputPeak { 0.0f };
    std::atomic<float> outputPeak { 0.0f };
    juce::AudioBuffer<float> dryBuffer;
    juce::SmoothedValue<float> mixSm;
    atone::SimpleLimiter safety;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AetherToneProcessor)
};
