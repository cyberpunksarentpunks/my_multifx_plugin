#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class ModuleCard : public juce::Component,
                   public juce::DragAndDropTarget
{
public:
    ModuleCard(AetherToneProcessor& p, juce::ValueTree state, int index);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    bool isInterestedInDragSource(const SourceDetails&) override { return true; }
    void itemDropped(const SourceDetails& drag) override;

    juce::String uuid;
    juce::String typeId;
    int index = 0;
    bool selected = false;

    std::function<void(int)> onSelect;
    std::function<void(int from, int to)> onMove;
    std::function<void()> onChanged;

private:
    AetherToneProcessor& processor;
    juce::TextButton bypass { "B" };
    juce::TextButton dup { "+" };
    juce::TextButton remove { "×" };
    juce::Colour accent;
    juce::String title;
    bool endpoint = false;
};

class ChainStrip : public juce::Component,
                   public juce::DragAndDropContainer
{
public:
    explicit ChainStrip(AetherToneProcessor& p);
    void resized() override;
    void paint(juce::Graphics& g) override;
    void rebuild();
    juce::String selectedUuid;
    std::function<void()> onSelectionChanged;

private:
    AetherToneProcessor& processor;
    juce::OwnedArray<ModuleCard> cards;
    juce::TextButton addButton { "+" };
    juce::Viewport viewport;
    juce::Component content;
};
