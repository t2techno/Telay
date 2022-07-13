/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TelayAudioProcessorEditor::TelayAudioProcessorEditor(TelayAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p), valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);

    // DryWet
    createLabelAndSlider(&mDryWetLabel, "Dry/Wet", &mDryWetSlider,
        0, 0, 100, 100);
    mDryWetAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "DryWet", mDryWetSlider));

    // Feedback
    createLabelAndSlider(&mFeedbackLabel, "Feedback", &mFeedbackSlider,
        100, 0, 100, 100);
    mFeedbackAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "Feedback", mFeedbackSlider));

    // Depth
    createLabelAndSlider(&mDepthLabel, "Depth", &mDepthSlider,
        200, 0, 100, 100);
    mDepthAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "Depth", mDepthSlider));

    // Rate
    createLabelAndSlider(&mRateLabel, "Rate", &mRateSlider,
        300, 0, 100, 100);
    mRateAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "Rate", mRateSlider));

    // PhaseOffset
    createLabelAndSlider(&mPhaseOffsetLabel, "PhaseOffset", &mPhaseOffsetSlider,
        0, 100, 100, 100);
    mPhaseOffsetAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "PhaseOffset", mPhaseOffsetSlider));

    mType.addItem("Delay", 1);
    mType.addItem("Chorus", 2);
    mType.addItem("Flanger", 3);

    mType.setBounds(100, 100, 100, 30);

    addAndMakeVisible(mType);
    mTypeAttach.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(vts, "Type", mType));
}


void TelayAudioProcessorEditor::createLabelAndSlider(juce::Label* label, std::string labelText, juce::Slider* slider,
    int x, int y, int width, int height) {
    label->setText(labelText, juce::dontSendNotification);
    addAndMakeVisible(label);

    addAndMakeVisible(slider);
    slider->setBounds(x, y, width, height);
    slider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    slider->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
}


TelayAudioProcessorEditor::~TelayAudioProcessorEditor()
{}

//==============================================================================
void TelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Telay", getLocalBounds(), juce::Justification::centred, 1);
}

void TelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}