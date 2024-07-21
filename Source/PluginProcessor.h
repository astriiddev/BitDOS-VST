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

    void setBitSet(BitSelect select, int i)
    {
        const char bit[] = { 'B', 'I', 'T', static_cast<char>(i + 0x31), '\0' };

        APVTS.getParameter(bit)->beginChangeGesture();
        APVTS.getParameterAsValue(bit).setValue(select);
        APVTS.getParameter(bit)->endChangeGesture();
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
        APVTS.getParameter("PREGAIN")->beginChangeGesture();

        if(gain >= 2.0) APVTS.getParameterAsValue("PREGAIN").setValue(2.0f);
        else if (gain <= 0.0) APVTS.getParameterAsValue("PREGAIN").setValue(0.0f);
        else APVTS.getParameterAsValue("PREGAIN").setValue(gain);

        APVTS.getParameter("PREGAIN")->endChangeGesture();
    }

    std::atomic<float>& getPostGain() { return postGain; }

    void setPostGain(float gain)
    {
        APVTS.getParameter("POSTGAIN")->beginChangeGesture();

        if (gain >= 2.0) APVTS.getParameterAsValue("POSTGAIN").setValue(2.0f);
        else if (gain <= 0.0) APVTS.getParameterAsValue("POSTGAIN").setValue(0.0f);
        else APVTS.getParameterAsValue("POSTGAIN").setValue(gain);

        APVTS.getParameter("POSTGAIN")->endChangeGesture();
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

    std::unique_ptr<juce::AudioParameterInt> createParam(const juce::String& name, const int min, const int max, const int def);
    std::unique_ptr<juce::AudioParameterFloat> createParam(const juce::String &name, const float& min, const float& max, const float& inc, const float def);

    void readBits(const int bit, const BitSelect select);

    bool bitMode{ true };

    float currentSample{ 0.0f }, muteCounter{ 0.0f };

    std::atomic<float> preGain { 1.0f }, postGain{ 1.0f }, blend   { 1.0f };
    std::atomic<bool> signedMode{ false }, isBypassed{ false }, paramsUpdated{ false };
    std::atomic<uint_fast8_t> bitInvert { 0 }, bitZeroed { 0 }, bitSample { 0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitDosAudioProcessor)
};
