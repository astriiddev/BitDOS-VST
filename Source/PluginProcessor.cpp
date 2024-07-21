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
    return samp > 1 ? 1.0f : samp < -1 ? -1.0f : samp;
}

void BitDosAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    const float* inL = buffer.getReadPointer(0);
    const float* inR = totalNumInputChannels > 1 ? buffer.getReadPointer(1) : nullptr;

    float* outL = buffer.getWritePointer(0);
    float* outR = totalNumOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    int numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (isBypassed) return;
    
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

    for(int i = 0; i < xml->getNumChildElements(); i++)
    {
        juce::String attrib(xml->getChildElement(i)->getStringAttribute("id"));
        if(attrib.isEmpty()) continue;
        if(!attrib.contains(" ")) continue;
        xml->removeChildElement(xml->getChildElement(i), true);
    }

    copyXmlToBinary(*xml, destData);
}

void BitDosAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() == nullptr) return;
    if (!xmlState->hasTagName(APVTS.state.getType())) return;

    APVTS.replaceState(juce::ValueTree::fromXml(*xmlState));

    for(int i = 0; i < APVTS.state.getNumChildren(); i++)
    {
        juce::ValueTree param = APVTS.state.getChild(i);
        if (param.getNumProperties() < 2) continue;
        valueTreePropertyChanged(param, param.getPropertyName(1));
    }

    signedMode = APVTS.state.getProperty("mode").operator bool();
    isBypassed = APVTS.state.getProperty("bypass").operator bool();
}

std::unique_ptr<juce::AudioParameterInt> BitDosAudioProcessor::createParam(const juce::String& name, const int min, const int max, const int def)
{
    return std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ name.toUpperCase(), 1 }, name, min, max, def);
}

std::unique_ptr<juce::AudioParameterFloat> BitDosAudioProcessor::createParam(const juce::String &name, const float& min, const float& max, const float& inc, const float def)
{
    return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name.toUpperCase(), 1 }, name, juce::NormalisableRange<float>(min, max, inc), def);
}

juce::AudioProcessorValueTreeState::ParameterLayout BitDosAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters(11);

    for (int i = 7; i >= 0; i--)
    {
        const char bitTxt[] = { 'B', 'i', 't', static_cast<char>(i + 0x31), '\0' };
        parameters.operator[](i) = createParam(juce::String(bitTxt, 5), 1, 3, 1);
    }

    parameters.operator[](8)  = createParam("PreGain",  0.0f, 2.0f, 0.001f, 1.0f);
    parameters.operator[](9)  = createParam("PostGain", 0.0f, 2.0f, 0.001f, 1.0f);
    parameters.operator[](10) = createParam("Blend",    0.0f, 2.0f, 0.001f, 1.0f);

    return  { parameters.begin(), parameters.end() };
}

void BitDosAudioProcessor::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    const char* changedParam = treeWhosePropertyHasChanged.getProperty(treeWhosePropertyHasChanged.getPropertyName(0)).toString().toRawUTF8();
    const juce::var paramVal = treeWhosePropertyHasChanged.getProperty(property);

    if(std::strstr(changedParam, "BIT") != nullptr)
    {
        const int bit = changedParam[3] == ' ' ? changedParam[4] : changedParam[3];
        readBits(bit - 0x31, static_cast<BitSelect>(paramVal.operator int()));
        return;
    }

    if(std::strstr(changedParam, "PRE") != nullptr)
    {
        preGain.store(paramVal.operator float());
        return;
    }

    if(std::strstr(changedParam, "POST") != nullptr)
    {
        postGain.store(paramVal.operator float());
        return;
    }

    if(std::strstr(changedParam, "BLEND") != nullptr)
    {
        blend.store(paramVal.operator float());
        return;
    }
}

void BitDosAudioProcessor::readBits(const int bit, const BitSelect select)
{
    bitSet[bit] = select;

    switch (bitSet[bit])
    {
    case (NORMAL_BIT):

        resetBit(bit);
        break;

    case (INVERT_BIT):

        setBitInvert(bit);
        break;

    case (HARD_0_BIT):

        setBitZeroed(bit);
        break;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitDosAudioProcessor();
}
