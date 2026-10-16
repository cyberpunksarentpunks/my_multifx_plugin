#include "FactoryPresets.h"
#include "../dsp/ModuleFactory.h"

static juce::ValueTree mod(const juce::String& type, std::initializer_list<std::pair<const char*, float>> params, bool bypass = false)
{
    auto m = createModule(type);
    juce::ValueTree t = m->saveState();
    t.setProperty("bypass", bypass, nullptr);
    for (auto& kv : params)
        t.setProperty(kv.first, kv.second, nullptr);
    return t;
}

static FactoryPreset makePreset(const juce::String& name, std::initializer_list<juce::ValueTree> modules)
{
    FactoryPreset p;
    p.name = name;
    p.chain = juce::ValueTree("CHAIN");
    for (auto m : modules)
        p.chain.appendChild(m, nullptr);
    return p;
}

juce::Array<FactoryPreset> getFactoryPresets()
{
    juce::Array<FactoryPreset> list;

    list.add(makePreset("Dreamscape Clean", {
        mod("input", { { "gain", 2.0f }, { "saturate", 0.12f } }),
        mod("modulation", { { "type", 0.0f }, { "rate", 0.35f }, { "depth", 0.4f }, { "mix", 0.45f }, { "feedback", 0.0f } }),
        mod("delay", { { "timeMs", 480.0f }, { "sync", 0.0f }, { "feedback", 0.42f }, { "mix", 0.32f }, { "mode", 1.0f }, { "hp", 120.0f }, { "lp", 5500.0f } }),
        mod("reverb", { { "decay", 0.82f }, { "predelay", 28.0f }, { "damp", 0.28f }, { "size", 1.25f }, { "width", 1.3f }, { "mix", 0.4f }, { "shimmer", 0.08f } }),
        mod("eq", { { "highGain", 2.0f }, { "lowGain", -1.0f }, { "midGain", -0.5f } }),
        mod("output", { { "gain", 0.0f }, { "width", 1.15f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Huge Ambient Clean", {
        mod("input", { { "gain", 1.5f }, { "saturate", 0.08f } }),
        mod("delay", { { "timeMs", 620.0f }, { "feedback", 0.48f }, { "mix", 0.38f }, { "mode", 1.0f }, { "lp", 4800.0f } }),
        mod("reverb", { { "decay", 0.9f }, { "predelay", 40.0f }, { "damp", 0.22f }, { "size", 1.6f }, { "width", 1.45f }, { "mix", 0.52f }, { "shimmer", 0.12f } }),
        mod("ambient", { { "volume", -14.0f }, { "mix", 0.28f }, { "cutoff", 1200.0f }, { "pitch", 55.0f }, { "track", 0.55f } }),
        mod("eq", { { "lowGain", -2.5f }, { "highGain", 1.5f }, { "hmGain", 1.0f } }),
        mod("output", { { "width", 1.25f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Guitar-to-Synth Bloom", {
        mod("input", { { "gain", 2.2f }, { "saturate", 0.12f } }),
        mod("granular", { { "density", 2.8f }, { "grainSize", 120.0f }, { "pitch", 7.0f }, { "spread", 0.7f }, { "mix", 0.46f }, { "feedback", 0.24f }, { "cutoff", 2600.0f }, { "shimmer", 0.18f } }),
        mod("shoegaze", { { "drive", 0.8f }, { "mix", 0.55f }, { "feedback", 0.62f }, { "shimmer", 0.4f }, { "cutoff", 2400.0f }, { "spread", 0.7f }, { "damp", 0.3f }, { "pan", 0.0f } }),
        mod("reverb", { { "decay", 0.92f }, { "predelay", 38.0f }, { "damp", 0.22f }, { "size", 1.75f }, { "width", 1.4f }, { "mix", 0.48f }, { "shimmer", 0.34f } }),
        mod("eq", { { "lowGain", -2.0f }, { "midGain", 1.0f }, { "highGain", 1.5f } }),
        mod("output", { { "gain", -1.0f }, { "width", 1.35f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Shimmering Cathedral", {
        mod("input", { { "gain", 0.0f }, { "saturate", 0.05f } }),
        mod("reverb", { { "decay", 0.94f }, { "predelay", 55.0f }, { "damp", 0.18f }, { "size", 1.85f }, { "width", 1.5f }, { "mix", 0.58f }, { "shimmer", 0.55f } }),
        mod("delay", { { "timeMs", 900.0f }, { "feedback", 0.3f }, { "mix", 0.18f }, { "mode", 0.0f }, { "lp", 7000.0f } }),
        mod("eq", { { "lowGain", -3.0f }, { "highGain", 3.0f }, { "hmFreq", 3200.0f }, { "hmGain", 1.5f } }),
        mod("output", { { "gain", -1.5f }, { "width", 1.4f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Infinite Delay", {
        mod("input", { { "gain", 0.0f } }),
        mod("delay", { { "timeMs", 375.0f }, { "sync", 1.0f }, { "division", 6.0f }, { "feedback", 0.92f }, { "mix", 0.55f }, { "mode", 1.0f }, { "hp", 180.0f }, { "lp", 4200.0f }, { "freeze", 0.0f } }),
        mod("eq", { { "highGain", -2.0f }, { "lowGain", -1.0f } }),
        mod("reverb", { { "decay", 0.45f }, { "mix", 0.12f }, { "size", 0.8f } }),
        mod("output", { { "gain", -1.0f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Post-Hardcore Intro", {
        mod("input", { { "gain", 4.0f }, { "saturate", 0.25f } }),
        mod("drive", { { "drive", 0.42f }, { "tone", 0.62f }, { "output", -3.0f }, { "character", 1.0f } }),
        mod("modulation", { { "type", 0.0f }, { "rate", 0.55f }, { "depth", 0.28f }, { "mix", 0.22f } }),
        mod("delay", { { "timeMs", 430.0f }, { "feedback", 0.36f }, { "mix", 0.28f }, { "mode", 1.0f } }),
        mod("reverb", { { "decay", 0.7f }, { "size", 1.1f }, { "mix", 0.28f }, { "predelay", 18.0f } }),
        mod("eq", { { "lowGain", 1.5f }, { "midGain", 1.0f }, { "highGain", 1.5f } }),
        mod("output", { { "gain", -1.0f }, { "width", 1.1f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Atmospheric Breakdown", {
        mod("input", { { "gain", 1.0f }, { "saturate", 0.15f } }),
        mod("drive", { { "drive", 0.22f }, { "tone", 0.4f }, { "character", 0.0f }, { "output", -2.0f } }),
        mod("ambient", { { "volume", -10.0f }, { "mix", 0.4f }, { "cutoff", 900.0f }, { "resonance", 2.4f }, { "pitch", 43.0f }, { "track", 0.7f }, { "modRate", 0.08f } }),
        mod("delay", { { "timeMs", 740.0f }, { "feedback", 0.55f }, { "mix", 0.4f }, { "mode", 1.0f }, { "hp", 150.0f }, { "lp", 3500.0f } }),
        mod("reverb", { { "decay", 0.88f }, { "size", 1.5f }, { "mix", 0.48f }, { "damp", 0.45f }, { "shimmer", 0.2f } }),
        mod("eq", { { "lowGain", -1.5f }, { "highGain", -1.0f }, { "midGain", -2.0f } }),
        mod("output", { { "width", 1.3f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Wide Modulated Clean", {
        mod("input", { { "gain", 2.5f } }),
        mod("modulation", { { "type", 0.0f }, { "rate", 0.7f }, { "depth", 0.55f }, { "mix", 0.55f } }),
        mod("modulation", { { "type", 3.0f }, { "rate", 0.25f }, { "depth", 0.4f }, { "mix", 0.3f }, { "feedback", 0.35f } }),
        mod("delay", { { "timeMs", 290.0f }, { "feedback", 0.22f }, { "mix", 0.2f }, { "mode", 1.0f } }),
        mod("reverb", { { "decay", 0.55f }, { "mix", 0.22f }, { "size", 0.95f }, { "width", 1.4f } }),
        mod("output", { { "width", 1.55f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Dark Drone", {
        mod("input", { { "gain", 0.0f }, { "saturate", 0.3f } }),
        mod("drive", { { "drive", 0.18f }, { "tone", 0.28f }, { "character", 0.0f } }),
        mod("ambient", { { "volume", -8.0f }, { "mix", 0.55f }, { "cutoff", 650.0f }, { "resonance", 3.2f }, { "pitch", 36.0f }, { "track", 0.35f }, { "modRate", 0.05f } }),
        mod("reverb", { { "decay", 0.93f }, { "size", 1.7f }, { "mix", 0.5f }, { "damp", 0.62f }, { "shimmer", 0.05f } }),
        mod("eq", { { "lowGain", 3.0f }, { "highGain", -6.0f }, { "hmGain", -3.0f } }),
        mod("output", { { "gain", -2.0f }, { "width", 0.85f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Reverse-Like Swells", {
        mod("input", { { "gain", 3.0f } }),
        mod("delay", { { "timeMs", 520.0f }, { "feedback", 0.25f }, { "mix", 0.62f }, { "mode", 2.0f }, { "hp", 90.0f }, { "lp", 8000.0f } }),
        mod("reverb", { { "decay", 0.8f }, { "predelay", 8.0f }, { "size", 1.3f }, { "mix", 0.42f } }),
        mod("eq", { { "highGain", 2.5f }, { "lowGain", -2.0f } }),
        mod("output", { { "width", 1.2f }, { "limit", 1.0f } })
    }));

    list.add(makePreset("Experimental Texture", {
        mod("input", { { "gain", 1.0f }, { "saturate", 0.4f } }),
        mod("drive", { { "drive", 0.3f }, { "character", 2.0f }, { "tone", 0.45f }, { "output", -6.0f } }),
        mod("modulation", { { "type", 2.0f }, { "rate", 0.18f }, { "depth", 0.6f }, { "mix", 0.4f }, { "feedback", 0.45f } }),
        mod("delay", { { "timeMs", 330.0f }, { "feedback", 0.7f }, { "mix", 0.45f }, { "mode", 1.0f }, { "hp", 220.0f }, { "lp", 2800.0f } }),
        mod("reverb", { { "decay", 0.86f }, { "shimmer", 0.4f }, { "size", 1.4f }, { "mix", 0.45f }, { "damp", 0.3f } }),
        mod("ambient", { { "mix", 0.33f }, { "cutoff", 1800.0f }, { "pitch", 60.0f }, { "resonance", 2.0f }, { "volume", -16.0f } }),
        mod("eq", { { "midGain", -3.0f }, { "highGain", 2.0f } }),
        mod("output", { { "gain", -2.5f }, { "width", 1.35f }, { "limit", 1.0f } })
    }));

    return list;
}

juce::File userPresetDirectory()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("AetherTone")
                   .getChildFile("Presets");
    dir.createDirectory();
    return dir;
}
