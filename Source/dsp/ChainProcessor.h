#pragma once

#include "FxModule.h"
#include <vector>
#include <memory>

class ChainProcessor
{
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, double bpm);

    void replaceWith(std::vector<std::unique_ptr<FxModule>> newModules);
    void addModule(std::unique_ptr<FxModule> module, int insertIndex = -1);
    void removeModule(const juce::String& uuid);
    void duplicateModule(const juce::String& uuid);
    void moveModule(int from, int to);
    void setBypassed(const juce::String& uuid, bool bypass);
    void setParam(const juce::String& uuid, const juce::Identifier& id, float value);

    juce::ValueTree saveState() const;
    void loadState(const juce::ValueTree& chainTree);

    std::vector<juce::ValueTree> snapshotForUi() const;
    int size() const;

    double hostBpm = 120.0;
    double sampleRate = 44100.0;
    int blockSize = 512;

    // Used by cabinet IR loading on the message thread.
    juce::CriticalSection lock;
    std::vector<std::unique_ptr<FxModule>> modules;

private:
    void ensureEndpoints();
    void prepareAll();
};
