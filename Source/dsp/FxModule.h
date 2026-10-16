#pragma once

#include <JuceHeader.h>
#include "DspUtils.h"
#include <array>

struct ParamDesc
{
    juce::Identifier id;
    juce::String label;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float defaultValue = 0.0f;
    float step = 0.0f;
    juce::String suffix;
    bool isChoice = false;
    juce::StringArray choices;
};

class FxModule
{
public:
    virtual ~FxModule() = default;

    juce::String uuid;
    juce::String typeId;
    juce::String displayName;
    juce::Colour accent { 0xff5ec8e3 };

    bool isBypassed() const noexcept { return bypassed.load(std::memory_order_relaxed); }
    void setBypassed(bool shouldBypass) noexcept { bypassed.store(shouldBypass, std::memory_order_relaxed); }

    virtual bool processWhenBypassed() const noexcept { return false; }
    virtual bool isFixedEndpoint() const noexcept { return false; }

    virtual void prepare(double sampleRate, int samplesPerBlock) = 0;
    virtual void reset() = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;

    const std::vector<ParamDesc>& getParamDescs() const { return paramDescs; }

    float getParam(const juce::Identifier& id) const
    {
        for (int i = 0; i < numParams; ++i)
            if (paramDescs[(size_t) i].id == id)
                return values[(size_t) i].load(std::memory_order_relaxed);
        return 0.0f;
    }

    void setParam(const juce::Identifier& id, float value)
    {
        for (int i = 0; i < numParams; ++i)
        {
            if (paramDescs[(size_t) i].id == id)
            {
                const auto& d = paramDescs[(size_t) i];
                values[(size_t) i].store(juce::jlimit(d.minValue, d.maxValue, value), std::memory_order_relaxed);
                return;
            }
        }
    }

    juce::ValueTree saveState() const
    {
        juce::ValueTree t("MODULE");
        t.setProperty("uuid", uuid, nullptr);
        t.setProperty("type", typeId, nullptr);
        t.setProperty("bypass", isBypassed(), nullptr);
        for (int i = 0; i < numParams; ++i)
            t.setProperty(paramDescs[(size_t) i].id, values[(size_t) i].load(), nullptr);
        extraState(t);
        return t;
    }

    void loadState(const juce::ValueTree& t)
    {
        uuid = t.getProperty("uuid").toString();
        setBypassed((bool) t.getProperty("bypass", false));
        for (int i = 0; i < numParams; ++i)
        {
            const auto& d = paramDescs[(size_t) i];
            if (t.hasProperty(d.id))
                values[(size_t) i].store((float) t.getProperty(d.id), std::memory_order_relaxed);
            else
                values[(size_t) i].store(d.defaultValue, std::memory_order_relaxed);
        }
        extraLoad(t);
    }

    virtual void extraState(juce::ValueTree&) const {}
    virtual void extraLoad(const juce::ValueTree&) {}

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

protected:
    void addParam(ParamDesc d)
    {
        jassert(numParams < (int) values.size());
        d.defaultValue = juce::jlimit(d.minValue, d.maxValue, d.defaultValue);
        values[(size_t) numParams].store(d.defaultValue, std::memory_order_relaxed);
        paramDescs.push_back(std::move(d));
        ++numParams;
    }

    float p(const char* id) const { return getParam(juce::Identifier(id)); }

    std::atomic<bool> bypassed { false };
    std::vector<ParamDesc> paramDescs;
    std::array<std::atomic<float>, 16> values {};
    int numParams = 0;
};
