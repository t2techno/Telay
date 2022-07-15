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
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    int comY = WINDOW_HEIGHT / 4;
    int comWFactor = WINDOW_WIDTH / 3.5;
    int comWidth = WINDOW_WIDTH / 4;
    int comHeight = WINDOW_HEIGHT / 4;

    // DryWet
    createLabelAndSlider(&mDryWetLabel, "Dry/Wet", &mDryWetSlider,
        .25* comWFactor, comY, comWidth, comHeight);
    mDryWetAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "DryWet", mDryWetSlider));

    // Feedback
    createLabelAndSlider(&mFeedbackLabel, "Feedback", &mFeedbackSlider,
        1.25* comWFactor, comY, comWidth, comHeight);
    mFeedbackAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "Feedback", mFeedbackSlider));

    // Depth
    createLabelAndSlider(&mDepthLabel, "Depth", &mDepthSlider,
        .25* comWFactor, comY*2.5, comWidth, comHeight);
    mDepthAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "Depth", mDepthSlider));

    // Rate
    createLabelAndSlider(&mRateLabel, "Rate", &mRateSlider,
        1.25*comWFactor, comY*2.5, comWidth, comHeight);
    mRateAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "Rate", mRateSlider));

    // PhaseOffset
    createLabelAndSlider(&mPhaseOffsetLabel, "PhaseOffset", &mPhaseOffsetSlider,
        2.25* comWFactor, comY*2.5, comWidth, comHeight);
    mPhaseOffsetAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "PhaseOffset", mPhaseOffsetSlider));

    addAndMakeVisible(&mTypeLabel);
    mTypeLabel.setText("Effect Type", juce::NotificationType::dontSendNotification);
    mTypeLabel.attachToComponent(&mType, false);

    mType.addItem("Delay", 1);
    mType.addItem("Chorus", 2);
    mType.addItem("Flanger", 3);

    mType.setBounds(.25*comWFactor, comY/3, comWidth, 30);

    addAndMakeVisible(mType);
    mTypeAttach.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(vts, "Type", mType));
}


void TelayAudioProcessorEditor::createLabelAndSlider(juce::Label* label, std::string labelText, juce::Slider* slider,
    int x, int y, int width, int height) {
    addAndMakeVisible(label);
    label->setText(labelText, juce::dontSendNotification);
    label->attachToComponent(slider, false);

    addAndMakeVisible(slider);
    slider->setBounds(x, y, width, height);
    slider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    slider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
}


TelayAudioProcessorEditor::~TelayAudioProcessorEditor()
{
    mDryWetAttach.reset();
    mFeedbackAttach.reset();
    mDepthAttach.reset();
    mRateAttach.reset();
    mPhaseOffsetAttach.reset();
    mTypeAttach.reset();
}

//==============================================================================
void TelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void TelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}