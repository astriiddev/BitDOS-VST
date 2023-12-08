/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

typedef enum
{
    NORMAL_BIT = 1,
    INVERT_BIT = 2,
    HARD_0_BIT = 3

} BitSelect;

class BitDosAudioProcessor  : public juce::AudioProcessor,
                              public juce::ValueTree::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    BitDosAudioProcessor();
    ~BitDosAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setBitInvert(int shift) { bitInvert |= (1 << shift); }
    void setBitZeroed(int shift) { bitZeroed |= (1 << shift); }

    void resetBit(int shift) 
    { 
        bitInvert &= ~(1 << shift); 
        bitZeroed &= ~(1 << shift); 
    }

    juce::AudioProcessorValueTreeState& getAPVTS() { return APVTS; }

    std::atomic<uint_fast8_t>& getBitSample() { return bitSample; }

    std::atomic<bool>& getBypassState() { return isBypassed; }
    void setBypassState() 
    { 
        isBypassed = !isBypassed; 
    
        APVTS.state.setProperty("bypass", (bool)isBypassed, nullptr);
    }

    void setBitSet(BitSelect select, int bit) 
    {
        std::stringstream ss;

        bitSet[bit] = select; 

        ss << "BIT " << (bit + 1);

        APVTS.getParameter(juce::String(ss.str()))->beginChangeGesture();

        APVTS.getParameterAsValue(juce::String(ss.str())).setValue(select);

        APVTS.getParameter(juce::String(ss.str()))->endChangeGesture();
    }

    BitSelect& getBitSet(int bit) { return bitSet[bit]; }

    std::atomic<bool>& inSignedMode() { return signedMode; }

    void setSignedMode() 
    {
        signedMode = !signedMode;

        APVTS.state.setProperty("mode", (bool)signedMode, nullptr);
    }

    bool& inBitMode() { return bitMode; }
    void setBitMode() { bitMode ^= 1; }

    std::atomic<float>& getPreGain() { return preGain; }

    void setPreGain(float gain)
    {
        APVTS.getParameter("PRE GAIN")->beginChangeGesture();

        if(gain >= 2.0) APVTS.getParameterAsValue("PRE GAIN").setValue(2.0f);

        else if (gain <= 0.0) APVTS.getParameterAsValue("PRE GAIN").setValue(0.0f);

        else APVTS.getParameterAsValue("PRE GAIN").setValue(gain);

        APVTS.getParameter("PRE GAIN")->endChangeGesture();
    }

    std::atomic<float>& getPostGain() { return postGain; }

    void setPostGain(float gain)
    {
        APVTS.getParameter("POST GAIN")->beginChangeGesture();

        if (gain >= 2.0) APVTS.getParameterAsValue("POST GAIN").setValue(2.0f);

        else if (gain <= 0.0) APVTS.getParameterAsValue("POST GAIN").setValue(0.0f);

        else APVTS.getParameterAsValue("POST GAIN").setValue(gain);

        APVTS.getParameter("POST GAIN")->endChangeGesture();
    }

    std::atomic<float>& getBlend() { return blend; }
    void setBlend(float vol)
    {
        APVTS.getParameter("BLEND")->beginChangeGesture();

        if (vol >= 1.0) APVTS.getParameterAsValue("BLEND").setValue(1.0f);

        else if (vol <= 0.0) APVTS.getParameterAsValue("BLEND").setValue(0.0f);

        else APVTS.getParameterAsValue("BLEND").setValue(vol);

        APVTS.getParameter("BLEND")->endChangeGesture();
    }

    float& getCurrentSample() { return currentSample; }

private:
    
    BitSelect bitSet[8] = { NORMAL_BIT, NORMAL_BIT, NORMAL_BIT, NORMAL_BIT,
                            NORMAL_BIT, NORMAL_BIT, NORMAL_BIT, NORMAL_BIT };

    juce::AudioProcessorValueTreeState APVTS;
    std::unique_ptr<juce::AudioProcessorValueTreeState::Listener> listener;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, 
                                  const juce::Identifier& property) override;

    void updateParams();
    void readBits();

    bool bitMode{ true };

    float currentSample{ 0.0f };

    std::atomic<float> preGain { 1.0f };
    std::atomic<float> postGain{ 1.0f };
    std::atomic<float> blend   { 1.0f };

    std::atomic<bool> signedMode{ false };
    std::atomic<bool> isBypassed{ false };
    std::atomic<bool> paramsUpdated{ false };

    std::atomic<uint_fast8_t> bitInvert { 0 };
    std::atomic<uint_fast8_t> bitZeroed { 0 };
    std::atomic<uint_fast8_t> bitSample { 0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitDosAudioProcessor)
};