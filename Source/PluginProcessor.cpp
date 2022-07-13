/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const std::string DRY_WET_ID = "DryWet";
const std::string FEEDBACK_ID = "Feedback";
const std::string DEPTH_ID = "Depth";
const std::string RATE_ID = "Rate";
const std::string PHASE_OFFSET_ID = "PhaseOffset";
const std::string TYPE_ID = "Type";

//==============================================================================
TelayAudioProcessor::TelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    params(*this, nullptr, juce::Identifier("Telay"),
        {
            std::make_unique<juce::AudioParameterFloat>(DRY_WET_ID, "Dry Wet", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>(FEEDBACK_ID, "Feedback", 0.0f, 0.98f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>(DEPTH_ID, "Depth", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>(RATE_ID, "Rate", 0.1f, 20.f, 10.f),
            std::make_unique<juce::AudioParameterFloat>(PHASE_OFFSET_ID, "Phase Offset", 0.0f, 1.f, 0.f),
            std::make_unique<juce::AudioParameterInt>(TYPE_ID, "Type", 0, 2, 0)
        }
    )
{
    // pointers to parameters
    mDryWetParameter = (juce::AudioParameterFloat*)params.getParameter(DRY_WET_ID);
    mFeedbackParameter = (juce::AudioParameterFloat*)params.getParameter(FEEDBACK_ID);
    mDepthParameter = (juce::AudioParameterFloat*)params.getParameter(DEPTH_ID);
    mRateParameter = (juce::AudioParameterFloat*)params.getParameter(RATE_ID);
    mPhaseOffsetParameter = (juce::AudioParameterFloat*)params.getParameter(PHASE_OFFSET_ID);
    mTypeParameter = (juce::AudioParameterInt*)params.getParameter(TYPE_ID);

    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    mDelayReadHead = 0.0;
    mLFOPhase = 0.0;
    mDelayTimeInSamples = 0;
    mFeedbackLeft = 0.0;
    mFeedbackRight = 0.0;
}

TelayAudioProcessor::~TelayAudioProcessor()
{
    mCircularBufferLeft.reset();
    mCircularBufferRight.reset();

    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    mDelayReadHead = 0.0;
    mDelayTimeInSamples = 0;
    mFeedbackLeft = 0.0;
    mFeedbackRight = 0.0;
    mTimeSmoothed = 0.0;
}

//==============================================================================
const juce::String TelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TelayAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TelayAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TelayAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TelayAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String TelayAudioProcessor::getProgramName(int index)
{
    return {};
}

void TelayAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void TelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // init data for current sample rate
    // reset phase and reheads

    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = std::unique_ptr<float[]>(new float[mCircularBufferLength]);
    }
    mCircularBufferLeft.reset(new float[mCircularBufferLength]);
    juce::zeromem(mCircularBufferLeft.get(), sizeof(float) * mCircularBufferLength);

    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = std::unique_ptr<float[]>(new float[mCircularBufferLength]);
    }
    mCircularBufferRight.reset(new float[mCircularBufferLength]);
    juce::zeromem(mCircularBufferRight.get(), sizeof(float) * mCircularBufferLength);

    mTimeSmoothed = *mRateParameter/10;
}

void TelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void TelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    float* outSamples;

    for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
        // Write to delay buffer
        mCircularBufferLeft.get()[mCircularBufferWriteHead] = leftChannel[sample] + mFeedbackLeft;
        mCircularBufferRight.get()[mCircularBufferWriteHead] = rightChannel[sample] + mFeedbackRight;

        // Iterate writeIndex, wrapping to 0 at the end (Circular)
        mCircularBufferWriteHead = (mCircularBufferWriteHead + 1) % mCircularBufferLength;

        // *mTypeParameter == 0 == false, Delay
        // *mTypeParameter > 0 == true, Chorus, Flanger
        if (*mTypeParameter) {
            outSamples = lfoSamplesOut(leftChannel, rightChannel, sample);
        }
        else {
            outSamples = delaySamplesOut(leftChannel, rightChannel, sample);
        }

        // Record Feedback
        mFeedbackLeft = outSamples[0] * *mFeedbackParameter;
        mFeedbackRight = outSamples[1] * *mFeedbackParameter;

        // mix dry/wet ratio of incoming sample with delayed sample
        float leftOutSample = buffer.getSample(0, sample) * (1 - *mDryWetParameter) + outSamples[0] * *mDryWetParameter;
        float rightOutSample = buffer.getSample(1, sample) * (1 - *mDryWetParameter) + outSamples[1] * *mDryWetParameter;

        // sample leaves
        buffer.setSample(0, sample, leftOutSample);
        buffer.setSample(1, sample, rightOutSample);
    }
}

float* TelayAudioProcessor::lfoSamplesOut(float* leftChannel, float* rightChannel, int sample) {

    // lfoOutRight is offset by mPhaseOffsetParameter
    float lfoPhaseRight = mLFOPhase + *mPhaseOffsetParameter;
    // wrap phase between 0 and 1
    if (lfoPhaseRight > 1) {
        lfoPhaseRight -= 1;
    }

    // lfoOut --> delayTime
    float lfoOutLeft = sin(2 * juce::float_Pi * mLFOPhase);
    float lfoOutRight = sin(2 * juce::float_Pi * lfoPhaseRight);

    // Scale by depth parameter
    lfoOutLeft *= *mDepthParameter;
    lfoOutRight *= *mDepthParameter;

    float lfoOutMappedLeft = 0.0;
    float lfoOutMappedRight = 0.0;

    if (*mTypeParameter == 1) {
        lfoOutMappedLeft = juce::jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
        lfoOutMappedRight = juce::jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
    }
    else {
        lfoOutMappedLeft = juce::jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
        lfoOutMappedRight = juce::jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
    }

    float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;
    float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;

    // iterate mLFOPhase
    mLFOPhase += *mRateParameter / getSampleRate();

    // wrap phase between 0 and 1
    if (mLFOPhase > 1) {
        mLFOPhase -= 1;
    }

    // calculate and wrap delayHeadLeft
    float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
    if (delayReadHeadLeft < 0) {
        delayReadHeadLeft += mCircularBufferLength;
    }

    // Floor of leftDelay, fract leftDelay, one up for interpolation
    int readHeadLeft_x = (int)delayReadHeadLeft;
    float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;
    int readHeadLeft_x1 = (readHeadLeft_x + 1) % mCircularBufferLength;

    //calculate and wrap delayHeadRight
    float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;
    if (delayReadHeadRight < 0) {
        delayReadHeadRight += mCircularBufferLength;
    }

    // Floor of rigfhtDelay, fract rightDelay, one up for interpolation
    int readHeadRight_x = (int)delayReadHeadRight;
    float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;
    int readHeadRight_x1 = (readHeadRight_x + 1) % mCircularBufferLength;

    // interpolated samples
    float delay_sample_left = lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
    float delay_sample_right = lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);
    return new float[2]{ delay_sample_left, delay_sample_right };
}

float* TelayAudioProcessor::delaySamplesOut(float* leftChannel, float* rightChannel, int sample) {
    // Smooth user input to prevent clicks and irregularities
    mTimeSmoothed = mTimeSmoothed - 0.0001 * (mTimeSmoothed - (*mRateParameter / 10));

    // Final delay time in samples
    mDelayTimeInSamples = getSampleRate() * mTimeSmoothed;

    // ReadHeadIndex with delay
    mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;

    // Wrap if less than 0
    if (mDelayReadHead < 0) {
        mDelayReadHead += mCircularBufferLength;
    }

    // floor of mDelayReadHead
    int readHead_x = (int)mDelayReadHead;

    // fract of mDelayReadHead
    float readHeadFloat = mDelayReadHead - readHead_x;

    // one sample up from mDelayReadHead for interpolation
    int readHead_x1 = (readHead_x + 1) % mCircularBufferLength;

    // interpolated samples
    float delay_sample_left = lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
    float delay_sample_right = lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);
    return new float[2]{ delay_sample_left, delay_sample_right };
}

//==============================================================================
bool TelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TelayAudioProcessor::createEditor()
{
    return new TelayAudioProcessorEditor(*this, params);
}

//==============================================================================
// Save plugin state when saving a DAW session
void TelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = params.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
    parameterPrint();
}

// Load plugin state when opening a saved DAW session
void TelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(params.state.getType()))
            params.replaceState(juce::ValueTree::fromXml(*xmlState));

    parameterPrint();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TelayAudioProcessor();
}

float TelayAudioProcessor::lin_interp(float inSampleX, float inSampleY, float inFloatPhase) {
    return (1 - inFloatPhase) * inSampleX + inFloatPhase * inSampleY;
}

void TelayAudioProcessor::parameterPrint() {
    DBG("DRY/WET: " << *mDryWetParameter);
    DBG("FEEDBACK: " << *mFeedbackParameter);
    DBG("DEPTH: " << *mDepthParameter);
    DBG("RATE: " << *mRateParameter);
    DBG("PHASEOFFSET: " << *mPhaseOffsetParameter);
    DBG("Type: " << *mTypeParameter);
}