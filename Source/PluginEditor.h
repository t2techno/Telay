/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH  480

//==============================================================================
/**
*/
class TelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TelayAudioProcessorEditor(TelayAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~TelayAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void createLabelAndSlider(juce::Label* label, std::string labelText, juce::Slider* slider,
        int x, int y, int width, int height);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TelayAudioProcessor& audioProcessor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TelayAudioProcessorEditor)
        juce::AudioProcessorValueTreeState& valueTreeState;
    juce::Slider mDryWetSlider;
    juce::Label mDryWetLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mDryWetAttach;

    juce::Slider mFeedbackSlider;
    juce::Label mFeedbackLabel;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> mFeedbackAttach;

    juce::Slider mDepthSlider;
    juce::Label mDepthLabel;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> mDepthAttach;

    juce::Slider mRateSlider;
    juce::Label mRateLabel;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> mRateAttach;

    juce::Slider mPhaseOffsetSlider;
    juce::Label mPhaseOffsetLabel;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> mPhaseOffsetAttach;

    juce::ComboBox mType;
    juce::Label mTypeLabel;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ComboBoxAttachment> mTypeAttach;
};
