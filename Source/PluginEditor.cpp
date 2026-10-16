#include "PluginEditor.h"

AetherToneEditor::AetherToneEditor(AetherToneProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p), chainStrip(p), modulePanel(p)
{
    setLookAndFeel(&lnf);
    setSize(1040, 680);
    setResizable(true, true);
    setResizeLimits(880, 560, 1600, 1000);

    titleLabel.setText("AetherTone", juce::dontSendNotification);
    titleLabel.setFont(juce::Font { juce::FontOptions(22.0f) }.boldened());
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe8eaef));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Guitar FX  ·  Ambient Processor", juce::dontSendNotification);
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8b90a0));
    addAndMakeVisible(subtitleLabel);

    presetBox.setTextWhenNothingSelected("Presets");
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id <= 0)
            return;
        if (id <= processor.factoryPresets.size())
            processor.loadFactoryPreset(id - 1);
        else
            processor.loadUserPreset(presetBox.getText());
    };
    addAndMakeVisible(presetBox);

    prevBtn.onClick = [this]
    {
        const int i = juce::jmax(0, processor.currentProgram - 1);
        processor.loadFactoryPreset(i);
    };
    nextBtn.onClick = [this]
    {
        const int i = juce::jmin(processor.factoryPresets.size() - 1, processor.currentProgram + 1);
        processor.loadFactoryPreset(i);
    };
    addAndMakeVisible(prevBtn);
    addAndMakeVisible(nextBtn);

    saveBtn.onClick = [this]
    {
        auto* w = new juce::AlertWindow("Save preset", "Name this effects chain.", juce::MessageBoxIconType::NoIcon);
        w->addTextEditor("name", processor.currentPresetName, "Preset name");
        w->addButton("Save", 1);
        w->addButton("Cancel", 0);
        w->enterModalState(true, juce::ModalCallbackFunction::create([this, w](int r)
        {
            if (r == 1)
            {
                auto name = w->getTextEditorContents("name");
                if (name.isNotEmpty())
                {
                    processor.saveUserPreset(name);
                    processor.currentPresetName = name;
                    refreshPresetBox();
                }
            }
        }), true);
    };
    addAndMakeVisible(saveBtn);

    exportBtn.onClick = [this]
    {
        auto fc = std::make_shared<juce::FileChooser>("Export preset", juce::File{}, "*.aethertone");
        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, fc](const juce::FileChooser& c)
                        {
                            auto f = c.getResult();
                            if (f != juce::File())
                                processor.exportCurrentPreset(f.hasFileExtension(".aethertone") ? f : f.withFileExtension(".aethertone"));
                        });
    };
    importBtn.onClick = [this]
    {
        auto fc = std::make_shared<juce::FileChooser>("Import preset", juce::File{}, "*.aethertone;*.xml");
        fc->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, fc](const juce::FileChooser& c)
                        {
                            auto f = c.getResult();
                            if (f.existsAsFile())
                                processor.loadPresetFile(f);
                        });
    };
    addAndMakeVisible(exportBtn);
    addAndMakeVisible(importBtn);

    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 16);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "mix", mixSlider);
    addAndMakeVisible(mixSlider);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, "bypass", bypass);
    addAndMakeVisible(bypass);

    chainStrip.onSelectionChanged = [this]
    {
        modulePanel.setModuleUuid(chainStrip.selectedUuid);
    };
    addAndMakeVisible(chainStrip);
    addAndMakeVisible(modulePanel);

    processor.addChangeListener(this);
    chainStrip.rebuild();
    refreshPresetBox();
    startTimerHz(30);
}

AetherToneEditor::~AetherToneEditor()
{
    processor.removeChangeListener(this);
    setLookAndFeel(nullptr);
}

void AetherToneEditor::refreshPresetBox()
{
    presetBox.clear(juce::dontSendNotification);
    int id = 1;
    presetBox.addSectionHeading("Factory");
    for (auto& p : processor.factoryPresets)
        presetBox.addItem(p.name, id++);
    const auto users = processor.listUserPresets();
    if (! users.isEmpty())
    {
        presetBox.addSeparator();
        presetBox.addSectionHeading("User");
        for (auto& n : users)
            presetBox.addItem(n, id++);
    }
    presetBox.setText(processor.currentPresetName, juce::dontSendNotification);
}

void AetherToneEditor::changeListenerCallback(juce::ChangeBroadcaster*)
{
    refreshPresetBox();
    chainStrip.rebuild();
}

void AetherToneEditor::timerCallback()
{
    repaint(getLocalBounds().removeFromRight(28).withTrimmedTop(0).withTrimmedBottom(0));
}

void AetherToneEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d0f14));
    auto header = getLocalBounds().removeFromTop(78).toFloat();
    juce::ColourGradient grad(juce::Colour(0xff141824), header.getTopLeft(),
                              juce::Colour(0xff0d0f14), header.getBottomLeft(), false);
    g.setGradientFill(grad);
    g.fillRect(header);

    auto meters = getLocalBounds().removeFromRight(26).reduced(4, 86);
    paintMeter(g, meters.removeFromLeft(9).toFloat(), processor.getInputPeak(), "IN");
    meters.removeFromLeft(4);
    paintMeter(g, meters.removeFromLeft(9).toFloat(), processor.getOutputPeak(), "OUT");
}

void AetherToneEditor::paintMeter(juce::Graphics& g, juce::Rectangle<float> area, float peak, const juce::String& name)
{
    g.setColour(juce::Colour(0xff1c2028));
    g.fillRoundedRectangle(area, 4.0f);
    const float db = juce::Decibels::gainToDecibels(peak, -48.0f);
    const float n = juce::jlimit(0.0f, 1.0f, juce::jmap(db, -48.0f, 0.0f, 0.0f, 1.0f));
    auto fill = area.removeFromBottom(area.getHeight() * n);
    g.setColour(n > 0.92f ? juce::Colour(0xffe07a5f) : juce::Colour(0xff5ec8e3));
    g.fillRoundedRectangle(fill, 3.0f);
    g.setColour(juce::Colour(0xff8b90a0));
    g.setFont(juce::Font { juce::FontOptions(9.0f) });
    g.drawText(name, juce::Rectangle<int>(area.getX() - 6, getHeight() - 78, 22, 14).toFloat(),
               juce::Justification::centred);
}

void AetherToneEditor::resized()
{
    auto r = getLocalBounds().reduced(16);
    r.removeFromRight(22);
    auto header = r.removeFromTop(58);
    titleLabel.setBounds(header.removeFromLeft(160).removeFromTop(28));
    subtitleLabel.setBounds(160 + 16, header.getY() + 28, 260, 18);
    bypass.setBounds(header.removeFromRight(90).reduced(4));
    mixSlider.setBounds(header.removeFromRight(70).reduced(0, -6));
    mixLabel.setBounds(mixSlider.getX(), header.getBottom() - 4, mixSlider.getWidth(), 16);
    exportBtn.setBounds(header.removeFromRight(72).reduced(4));
    importBtn.setBounds(header.removeFromRight(72).reduced(4));
    saveBtn.setBounds(header.removeFromRight(64).reduced(4));
    nextBtn.setBounds(header.removeFromRight(36).reduced(4));
    prevBtn.setBounds(header.removeFromRight(36).reduced(4));
    presetBox.setBounds(header.reduced(8, 10));

    r.removeFromTop(10);
    chainStrip.setBounds(r.removeFromTop(118));
    r.removeFromTop(12);
    modulePanel.setBounds(r);
}
