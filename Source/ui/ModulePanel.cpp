#include "ModulePanel.h"
#include "../dsp/ModuleFactory.h"
#include "../dsp/modules/CabinetModule.h"

ModulePanel::ModulePanel(AetherToneProcessor& p) : processor(p)
{
    loadIr.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser>("Load impulse response", juce::File{}, "*.wav;*.aif;*.aiff;*.flac");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                             [this, chooser](const juce::FileChooser& fc)
                             {
                                 auto file = fc.getResult();
                                 if (! file.existsAsFile())
                                     return;
                                 const juce::ScopedLock sl(processor.chain.lock);
                                 for (auto& m : processor.chain.modules)
                                 {
                                     if (auto* cab = dynamic_cast<CabinetModule*>(m.get()))
                                     {
                                         if (cab->uuid == uuid)
                                         {
                                             cab->loadImpulseResponse(file);
                                             irLabel.setText(file.getFileName(), juce::dontSendNotification);
                                             processor.currentPresetName = "Custom";
                                             processor.sendChangeMessage();
                                         }
                                     }
                                 }
                             });
    };
    addChildComponent(loadIr);
    irLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8b90a0));
    addChildComponent(irLabel);
}

void ModulePanel::setModuleUuid(const juce::String& newUuid)
{
    uuid = newUuid;
    auto snap = processor.chain.snapshotForUi();
    juce::ValueTree found;
    for (auto& t : snap)
        if (t.getProperty("uuid").toString() == uuid)
            found = t;
    rebuildControls(found);
}

void ModulePanel::rebuildControls(const juce::ValueTree& state)
{
    knobs.clear();
    loadIr.setVisible(false);
    irLabel.setVisible(false);
    title.clear();
    if (! state.isValid())
    {
        repaint();
        return;
    }

    title = displayNameForType(state.getProperty("type").toString());
    accent = accentForType(state.getProperty("type").toString());
    auto temp = createModule(state.getProperty("type").toString());
    if (temp == nullptr)
        return;
    temp->loadState(state);

    for (auto& desc : temp->getParamDescs())
    {
        Knob k;
        k.id = desc.id;
        k.label = std::make_unique<juce::Label>();
        k.label->setText(desc.label, juce::dontSendNotification);
        k.label->setJustificationType(juce::Justification::centred);
        k.label->setColour(juce::Label::textColourId, juce::Colour(0xff8b90a0));
        addAndMakeVisible(*k.label);

        if (desc.isChoice)
        {
            k.combo = std::make_unique<juce::ComboBox>();
            int idx = 1;
            for (auto& c : desc.choices)
                k.combo->addItem(c, idx++);
            k.combo->setSelectedItemIndex((int) std::round((float) state.getProperty(desc.id, desc.defaultValue)),
                                          juce::dontSendNotification);
            k.combo->onChange = [this, id = desc.id, combo = k.combo.get()]
            {
                processor.chain.setParam(uuid, id, (float) combo->getSelectedItemIndex());
                processor.currentPresetName = "Custom";
            };
            addAndMakeVisible(*k.combo);
        }
        else
        {
            k.slider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
            k.slider->setRange(desc.minValue, desc.maxValue, desc.step > 0.0f ? desc.step : 0.0);
            k.slider->setValue((float) state.getProperty(desc.id, desc.defaultValue), juce::dontSendNotification);
            k.slider->setTextValueSuffix(desc.suffix);
            k.slider->setColour(juce::Slider::rotarySliderFillColourId, accent);
            k.slider->onValueChange = [this, id = desc.id, sl = k.slider.get()]
            {
                processor.chain.setParam(uuid, id, (float) sl->getValue());
                processor.currentPresetName = "Custom";
            };
            addAndMakeVisible(*k.slider);
        }
        knobs.push_back(std::move(k));
    }

    if (state.getProperty("type").toString() == "cabinet")
    {
        loadIr.setVisible(true);
        irLabel.setVisible(true);
        irLabel.setText(state.getProperty("irPath", "No IR loaded — bypassed until you load a WAV").toString(),
                        juce::dontSendNotification);
    }
    resized();
    repaint();
}

void ModulePanel::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff161920));
    g.fillRoundedRectangle(r, 14.0f);
    g.setColour(accent.withAlpha(0.55f));
    g.drawRoundedRectangle(r.reduced(0.5f), 14.0f, 1.0f);
    g.setColour(juce::Colour(0xffe8eaef));
    g.setFont(juce::Font { juce::FontOptions(16.0f) }.boldened());
    g.drawText(title.isEmpty() ? "Select a module" : title,
               getLocalBounds().removeFromTop(36), juce::Justification::centred);
}

void ModulePanel::resized()
{
    auto r = getLocalBounds().reduced(16).withTrimmedTop(36);
    if (loadIr.isVisible())
    {
        auto row = r.removeFromBottom(36);
        loadIr.setBounds(row.removeFromLeft(120));
        irLabel.setBounds(row.reduced(8, 0));
    }

    const int count = (int) knobs.size();
    if (count == 0)
        return;
    const int cols = juce::jmin(count, 6);
    const int rows = (count + cols - 1) / cols;
    const int w = r.getWidth() / cols;
    const int h = r.getHeight() / juce::jmax(1, rows);
    for (int i = 0; i < count; ++i)
    {
        const int col = i % cols;
        const int row = i / cols;
        auto cell = juce::Rectangle<int>(r.getX() + col * w, r.getY() + row * h, w, h).reduced(6);
        knobs[(size_t) i].label->setBounds(cell.removeFromTop(18));
        if (knobs[(size_t) i].combo != nullptr)
            knobs[(size_t) i].combo->setBounds(cell.removeFromTop(32).reduced(8, 0));
        else if (knobs[(size_t) i].slider != nullptr)
            knobs[(size_t) i].slider->setBounds(cell);
    }
}
