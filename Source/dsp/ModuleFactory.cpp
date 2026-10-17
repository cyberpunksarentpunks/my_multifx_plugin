#include "ModuleFactory.h"
#include "modules/InputModule.h"
#include "modules/DriveModule.h"
#include "modules/ModulationModule.h"
#include "modules/DelayModule.h"
#include "modules/ReverbModule.h"
#include "modules/AmbientModule.h"
#include "modules/GranularModule.h"
#include "modules/ShoegazeModule.h"
#include "modules/CosmicSpaceModule.h"
#include "modules/ConsolePreampModule.h"
#include "modules/EqModule.h"
#include "modules/CabinetModule.h"
#include "modules/OutputModule.h"

std::unique_ptr<FxModule> createModule(const juce::String& typeId)
{
    std::unique_ptr<FxModule> m;
    if (typeId == "input") m = std::make_unique<InputModule>();
    else if (typeId == "drive") m = std::make_unique<DriveModule>();
    else if (typeId == "modulation") m = std::make_unique<ModulationModule>();
    else if (typeId == "delay") m = std::make_unique<DelayModule>();
    else if (typeId == "reverb") m = std::make_unique<ReverbModule>();
    else if (typeId == "ambient") m = std::make_unique<AmbientModule>();
    else if (typeId == "granular") m = std::make_unique<GranularModule>();
    else if (typeId == "shoegaze") m = std::make_unique<ShoegazeModule>();
    else if (typeId == "cosmicspace") m = std::make_unique<CosmicSpaceModule>();
    else if (typeId == "consolepreamp") m = std::make_unique<ConsolePreampModule>();
    else if (typeId == "eq") m = std::make_unique<EqModule>();
    else if (typeId == "cabinet") m = std::make_unique<CabinetModule>();
    else if (typeId == "output") m = std::make_unique<OutputModule>();
    else return nullptr;

    if (m->uuid.isEmpty())
        m->uuid = juce::Uuid().toDashedString();
    return m;
}

juce::StringArray allModuleTypeIds()
{
    return { "drive", "modulation", "delay", "reverb", "ambient", "granular", "shoegaze", "cosmicspace", "consolepreamp", "eq", "cabinet" };
}

juce::String displayNameForType(const juce::String& typeId)
{
    auto m = createModule(typeId);
    return m != nullptr ? m->displayName : typeId;
}

juce::Colour accentForType(const juce::String& typeId)
{
    auto m = createModule(typeId);
    return m != nullptr ? m->accent : juce::Colours::grey;
}

bool isEndpointType(const juce::String& typeId)
{
    return typeId == "input" || typeId == "output";
}
