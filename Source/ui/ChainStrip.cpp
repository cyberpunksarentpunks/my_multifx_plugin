#include "ChainStrip.h"
#include "../dsp/ModuleFactory.h"

ModuleCard::ModuleCard(AetherToneProcessor& p, juce::ValueTree state, int indexIn)
    : processor(p)
{
    index = indexIn;
    uuid = state.getProperty("uuid").toString();
    typeId = state.getProperty("type").toString();
    title = displayNameForType(typeId);
    accent = accentForType(typeId);
    endpoint = isEndpointType(typeId);

    bypass.setClickingTogglesState(true);
    bypass.setToggleState((bool) state.getProperty("bypass", false), juce::dontSendNotification);
    bypass.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffe07a5f));
    bypass.onClick = [this]
    {
        processor.chain.setBypassed(uuid, bypass.getToggleState());
        processor.currentPresetName = "Custom";
        processor.sendChangeMessage();
    };
    addAndMakeVisible(bypass);

    dup.setVisible(! endpoint);
    dup.onClick = [this]
    {
        processor.chain.duplicateModule(uuid);
        processor.currentPresetName = "Custom";
        if (onChanged) onChanged();
    };
    addAndMakeVisible(dup);

    remove.setVisible(! endpoint);
    remove.onClick = [this]
    {
        processor.chain.removeModule(uuid);
        processor.currentPresetName = "Custom";
        if (onChanged) onChanged();
    };
    addAndMakeVisible(remove);
}

void ModuleCard::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(3.0f);
    g.setColour(selected ? accent.withAlpha(0.22f) : juce::Colour(0xff1c2028));
    g.fillRoundedRectangle(r, 10.0f);
    g.setColour(selected ? accent : juce::Colour(0xff2c3340));
    g.drawRoundedRectangle(r, 10.0f, selected ? 1.6f : 1.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(r.removeFromLeft(4.0f).reduced(0, 8.0f), 2.0f);
    g.setColour(juce::Colour(0xffe8eaef));
    g.setFont(juce::Font { juce::FontOptions(13.0f) }.boldened());
    g.drawText(title, getLocalBounds().reduced(12, 10).withTrimmedTop(18), juce::Justification::centredTop);
    if (bypass.getToggleState())
    {
        g.setColour(juce::Colour(0xffe07a5f));
        g.setFont(juce::Font { juce::FontOptions(10.0f) });
        g.drawText("BYPASSED", getLocalBounds().reduced(8), juce::Justification::centredBottom);
    }
}

void ModuleCard::resized()
{
    auto r = getLocalBounds().reduced(8);
    bypass.setBounds(r.removeFromTop(22).removeFromLeft(28));
    auto top = getLocalBounds().reduced(8).removeFromTop(22);
    remove.setBounds(top.removeFromRight(22));
    dup.setBounds(top.removeFromRight(22).translated(-4, 0));
}

void ModuleCard::mouseDown(const juce::MouseEvent&)
{
    if (onSelect) onSelect(index);
}

void ModuleCard::mouseDrag(const juce::MouseEvent& e)
{
    if (endpoint)
        return;
    if (e.getDistanceFromDragStart() > 6)
        if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
            container->startDragging("module:" + juce::String(index), this);
}

void ModuleCard::itemDropped(const SourceDetails& drag)
{
    const int from = drag.description.toString().fromFirstOccurrenceOf(":", false, false).getIntValue();
    if (onMove) onMove(from, index);
}

ChainStrip::ChainStrip(AetherToneProcessor& p) : processor(p)
{
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
    viewport.setScrollBarsShown(false, true);
    addButton.setTooltip("Add effect");
    addButton.onClick = [this]
    {
        juce::PopupMenu m;
        for (auto& type : allModuleTypeIds())
            m.addItem(displayNameForType(type), [this, type]
            {
                processor.chain.addModule(createModule(type));
                processor.currentPresetName = "Custom";
                rebuild();
                processor.sendChangeMessage();
            });
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&addButton));
    };
    addAndMakeVisible(addButton);
}

void ChainStrip::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff12151b));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 12.0f);
}

void ChainStrip::resized()
{
    auto r = getLocalBounds().reduced(8);
    addButton.setBounds(r.removeFromRight(44).reduced(0, 10));
    viewport.setBounds(r);
    content.setBounds(0, 0, juce::jmax(r.getWidth(), cards.size() * 148 + 12), r.getHeight());
    int x = 6;
    for (auto* c : cards)
    {
        c->setBounds(x, 6, 140, content.getHeight() - 12);
        x += 148;
    }
}

void ChainStrip::rebuild()
{
    cards.clear();
    auto snap = processor.chain.snapshotForUi();
    bool keep = false;
    for (int i = 0; i < (int) snap.size(); ++i)
    {
        auto* card = cards.add(new ModuleCard(processor, snap[(size_t) i], i));
        if (card->uuid == selectedUuid)
            keep = true;
        card->onSelect = [this](int idx)
        {
            if (juce::isPositiveAndBelow(idx, cards.size()))
            {
                selectedUuid = cards[idx]->uuid;
                for (int j = 0; j < cards.size(); ++j)
                    cards[j]->selected = (j == idx);
                repaint();
                if (onSelectionChanged) onSelectionChanged();
            }
        };
        card->onMove = [this](int from, int to)
        {
            processor.chain.moveModule(from, to);
            processor.currentPresetName = "Custom";
            rebuild();
            processor.sendChangeMessage();
        };
        card->onChanged = [this]
        {
            rebuild();
            processor.sendChangeMessage();
        };
        content.addAndMakeVisible(card);
    }
    if (! keep && ! cards.isEmpty())
        selectedUuid = cards.getFirst()->uuid;
    for (auto* c : cards)
        c->selected = (c->uuid == selectedUuid);
    resized();
    if (onSelectionChanged) onSelectionChanged();
}
