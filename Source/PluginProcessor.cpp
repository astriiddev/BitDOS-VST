/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BitDosAudioProcessor::BitDosAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), APVTS(*this, nullptr, "Parameters", createParameters())
#endif
{
    APVTS.state.addListener(this);
}

BitDosAudioProcessor::~BitDosAudioProcessor()
{
}

//==============================================================================
const juce::String BitDosAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BitDosAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BitDosAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BitDosAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BitDosAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BitDosAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BitDosAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BitDosAudioProcessor::setCurrentProgram (int)
{
}

const juce::String BitDosAudioProcessor::getProgramName (int)
{
    return {};
}

void BitDosAudioProcessor::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void BitDosAudioProcessor::prepareToPlay (double, int)
{
}

void BitDosAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BitDosAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

static int_fast8_t clamp_int8(const int_fast16_t samp)
{
    if ((int_fast8_t)samp == samp) return (int_fast8_t)samp;
    else return (int_fast8_t)(INT8_MAX ^ (samp >> 4));
}

static uint_fast8_t clamp_uint8(const int_fast16_t samp)
{
    if ((uint_fast8_t)samp == samp) return (uint_fast8_t)samp;
    else return (uint_fast8_t)(UINT8_MAX ^ (samp >> 4));
}

static float clamp_float(const float samp)
{
    return (abs(samp) < 1.0) ? samp : (samp < 0) ? -1 : 1 ;
}

void BitDosAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (paramsUpdated) updateParams();
    if (isBypassed) return;
    
    int numSamples = buffer.getNumSamples();

    const float* inL = buffer.getReadPointer(0);
    const float* inR = totalNumInputChannels > 1 ? buffer.getReadPointer(1) : nullptr;

    float* outL = buffer.getWritePointer(0);
    float* outR = totalNumOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    while (--numSamples >= 0)
    {
        float out[2] = { 0.f, 0.f };

        float sampL = *inL++;
        float sampR = inR == nullptr ? sampL : *inR++;

        uint8_t bitSampleR = 0;

        currentSample = sampL == 0.f ? sampR : sampL;

        if (std::abs(sampL) <= 0.01f && std::abs(sampR) <= 0.01f) 
        { 
            bitSample = 0; 
            muteCounter += 1.f; 
        }
        else if (bitZeroed == 255) 
        {
            bitSample = 0; 
            muteCounter += 1.f; 
        }
        else muteCounter = 0.f;
                
        if (signedMode)
        {
            bitSample  = (clamp_int8((int_fast16_t)((sampL * preGain) * 127.5f)) ^ bitInvert) & ~bitZeroed;
            bitSampleR = (clamp_int8((int_fast16_t)((sampR * preGain) * 127.5f)) ^ bitInvert) & ~bitZeroed;

            out[0] = clamp_float(((blend * ((float)(int_fast8_t)bitSample  / 128)) + (sampL * -(blend - 1))) * postGain);
            out[1] = clamp_float(((blend * ((float)(int_fast8_t)bitSampleR / 128)) + (sampR * -(blend - 1))) * postGain);
        }
        else
        {
            bitSample  = (clamp_uint8((int_fast16_t)(((sampL * preGain) + 1.0) * 127.5f)) ^ bitInvert) & ~bitZeroed;
            bitSampleR = (clamp_uint8((int_fast16_t)(((sampR * preGain) + 1.0) * 127.5f)) ^ bitInvert) & ~bitZeroed;

            out[0] = clamp_float(((blend * (((float)bitSample  / 128) - 1.0f)) + (sampL * -(blend - 1))) * postGain);
            out[1] = clamp_float(((blend * (((float)bitSampleR / 128) - 1.0f)) + (sampR * -(blend - 1))) * postGain);
        }

        if (sampL == 0.f) bitSample = bitSampleR;

        if (outR == nullptr)
        {
            *outL++ = muteCounter >= 128.f ? (out[0] + out[1]) * 64.f / muteCounter : (out[0] + out[1]) * 0.5f;
        }
        else
        {
            *outL++ = muteCounter >= 128.f ? out[0] * 128.f / muteCounter : out[0];
            *outR++ = muteCounter >= 128.f ? out[1] * 128.f / muteCounter : out[1];
        }
    }
    
}

//==============================================================================
bool BitDosAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BitDosAudioProcessor::createEditor()
{
    return new BitDosAudioProcessorEditor (*this);
}

//==============================================================================
void BitDosAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = APVTS.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BitDosAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(APVTS.state.getType()))
        {
            APVTS.replaceState(juce::ValueTree::fromXml(*xmlState));

            updateParams();

            signedMode = (bool)APVTS.state.getProperty("mode");
            isBypassed = (bool)APVTS.state.getProperty("bypass");

        }
}

juce::AudioProcessorValueTreeState::ParameterLayout BitDosAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    for (int i = 8; i >= 1; i--)
    {
        const juce::String bit = "Bit " + juce::String(i + 1);

        parameters.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ bit.toUpperCase(), 1 },
            bit, 1, 3, 1));
    }

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "PRE GAIN", 1 }, "Pre Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "POST GAIN", 1 }, "Post Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "BLEND", 1 }, "Blend",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f));

    return  { parameters.begin(), parameters.end() };
}

void BitDosAudioProcessor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
{
    paramsUpdated = true;
}

void BitDosAudioProcessor::updateParams()
{
    readBits();

    preGain = APVTS.getRawParameterValue("PRE GAIN")->load();
    postGain = APVTS.getRawParameterValue("POST GAIN")->load();
    blend = APVTS.getRawParameterValue("BLEND")->load();

    paramsUpdated = false;
}

void BitDosAudioProcessor::readBits()
{
    for (int i = 7; i >= 0; i--)
    {
        const juce::String bit = "BIT " + juce::String(i + 1);

        bitSet[i] = (BitSelect)(int)APVTS.getRawParameterValue(bit)->load();

        switch (bitSet[i])
        {
        case (NORMAL_BIT):

            resetBit(i);
            break;

        case (INVERT_BIT):

            setBitInvert(i);
            break;

        case (HARD_0_BIT):

            setBitZeroed(i);
            break;
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitDosAudioProcessor();
}
