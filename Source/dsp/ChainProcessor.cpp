#include "ChainProcessor.h"
#include "ModuleFactory.h"
#include "modules/DelayModule.h"
#include "modules/InputModule.h"
#include "modules/OutputModule.h"
#include <algorithm>

void ChainProcessor::prepare(double sr, int spb)
{
    sampleRate = sr;
    blockSize = spb;
    const juce::ScopedLock sl(lock);
    prepareAll();
}

void ChainProcessor::prepareAll()
{
    for (auto& m : modules)
    {
        m->prepare(sampleRate, blockSize);
        m->reset();
    }
}

void ChainProcessor::reset()
{
    const juce::ScopedLock sl(lock);
    for (auto& m : modules)
        m->reset();
}

void ChainProcessor::process(juce::AudioBuffer<float>& buffer, double bpm)
{
    hostBpm = bpm;
    const juce::ScopedLock sl(lock);
    for (auto& m : modules)
    {
        if (auto* delay = dynamic_cast<DelayModule*>(m.get()))
            delay->hostBpm = bpm;

        if (m->isBypassed() && ! m->processWhenBypassed())
            continue;

        m->process(buffer);
    }
}

void ChainProcessor::ensureEndpoints()
{
    bool hasIn = false, hasOut = false;
    for (auto& m : modules)
    {
        hasIn = hasIn || m->typeId == "input";
        hasOut = hasOut || m->typeId == "output";
    }
    if (! hasIn)
        modules.insert(modules.begin(), createModule("input"));
    if (! hasOut)
        modules.push_back(createModule("output"));
}

void ChainProcessor::replaceWith(std::vector<std::unique_ptr<FxModule>> newModules)
{
    const juce::ScopedLock sl(lock);
    modules = std::move(newModules);
    ensureEndpoints();
    prepareAll();
}

void ChainProcessor::addModule(std::unique_ptr<FxModule> module, int insertIndex)
{
    if (module == nullptr)
        return;
    module->prepare(sampleRate, blockSize);
    const juce::ScopedLock sl(lock);
    ensureEndpoints();
    if (insertIndex < 0)
        insertIndex = (int) modules.size() - 1; // before output
    insertIndex = juce::jlimit(1, juce::jmax(1, (int) modules.size() - 1), insertIndex);
    modules.insert(modules.begin() + insertIndex, std::move(module));
}

void ChainProcessor::removeModule(const juce::String& uuid)
{
    const juce::ScopedLock sl(lock);
    modules.erase(std::remove_if(modules.begin(), modules.end(),
                                 [&](const auto& m) { return m->uuid == uuid && ! m->isFixedEndpoint(); }),
                  modules.end());
}

void ChainProcessor::duplicateModule(const juce::String& uuid)
{
    juce::ValueTree copy;
    int index = -1;
    {
        const juce::ScopedLock sl(lock);
        for (int i = 0; i < (int) modules.size(); ++i)
        {
            if (modules[(size_t) i]->uuid == uuid && ! modules[(size_t) i]->isFixedEndpoint())
            {
                copy = modules[(size_t) i]->saveState();
                index = i;
                break;
            }
        }
    }
    if (index < 0)
        return;
    auto m = createModule(copy.getProperty("type").toString());
    if (m == nullptr)
        return;
    copy.setProperty("uuid", juce::Uuid().toDashedString(), nullptr);
    m->loadState(copy);
    addModule(std::move(m), index + 1);
}

void ChainProcessor::moveModule(int from, int to)
{
    const juce::ScopedLock sl(lock);
    const int last = (int) modules.size() - 1;
    if (from <= 0 || from >= last || to <= 0 || to >= last || from == to)
        return;
    auto item = std::move(modules[(size_t) from]);
    modules.erase(modules.begin() + from);
    if (to > from)
        --to;
    modules.insert(modules.begin() + to, std::move(item));
}

void ChainProcessor::setBypassed(const juce::String& uuid, bool bypass)
{
    const juce::ScopedLock sl(lock);
    for (auto& m : modules)
        if (m->uuid == uuid)
            m->setBypassed(bypass);
}

void ChainProcessor::setParam(const juce::String& uuid, const juce::Identifier& id, float value)
{
    // atomics — lock-free for DSP params
    const juce::ScopedLock sl(lock);
    for (auto& m : modules)
        if (m->uuid == uuid)
            m->setParam(id, value);
}

juce::ValueTree ChainProcessor::saveState() const
{
    const juce::ScopedLock sl(lock);
    juce::ValueTree chain("CHAIN");
    for (auto& m : modules)
        chain.appendChild(m->saveState(), nullptr);
    return chain;
}

void ChainProcessor::loadState(const juce::ValueTree& chainTree)
{
    std::vector<std::unique_ptr<FxModule>> next;
    for (const auto& child : chainTree)
    {
        if (! child.hasType("MODULE"))
            continue;
        auto m = createModule(child.getProperty("type").toString());
        if (m == nullptr)
            continue;
        m->loadState(child);
        next.push_back(std::move(m));
    }
    replaceWith(std::move(next));
}

std::vector<juce::ValueTree> ChainProcessor::snapshotForUi() const
{
    const juce::ScopedLock sl(lock);
    std::vector<juce::ValueTree> out;
    out.reserve(modules.size());
    for (auto& m : modules)
        out.push_back(m->saveState());
    return out;
}

int ChainProcessor::size() const
{
    const juce::ScopedLock sl(lock);
    return (int) modules.size();
}
