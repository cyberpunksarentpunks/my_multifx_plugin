cmake_minimum_required(VERSION 3.22)

project(AetherTone VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Native Apple Silicon" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.8
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(AetherTone
    COMPANY_NAME "AetherTone"
    BUNDLE_ID com.aethertone.plugin
    PLUGIN_MANUFACTURER_CODE Aetn
    PLUGIN_CODE Atfx
    FORMATS VST3 Standalone
    PRODUCT_NAME "AetherTone"
    PLUGIN_NAME "AetherTone"
    DESCRIPTION "Guitar FX and ambient processor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "AetherTone Standalone needs microphone access to process live guitar input."
)

juce_generate_juce_header(AetherTone)

target_sources(AetherTone
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/dsp/ChainProcessor.cpp
        Source/dsp/ModuleFactory.cpp
        Source/dsp/modules/InputModule.cpp
        Source/dsp/modules/DriveModule.cpp
        Source/dsp/modules/ModulationModule.cpp
        Source/dsp/modules/DelayModule.cpp
        Source/dsp/modules/ReverbModule.cpp
        Source/dsp/modules/AmbientModule.cpp
        Source/dsp/modules/GranularModule.cpp
        Source/dsp/modules/ShoegazeModule.cpp
        Source/dsp/modules/CosmicSpaceModule.cpp
        Source/dsp/modules/EqModule.cpp
        Source/dsp/modules/CabinetModule.cpp
        Source/dsp/modules/OutputModule.cpp
        Source/ui/AetherLookAndFeel.cpp
        Source/ui/ChainStrip.cpp
        Source/ui/ModulePanel.cpp
        Source/presets/FactoryPresets.cpp
)

target_include_directories(AetherTone
    PRIVATE
        Source
)

target_compile_definitions(AetherTone
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(AetherTone
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(APPLE)
    target_compile_options(AetherTone PRIVATE -Wno-error)
endif()
