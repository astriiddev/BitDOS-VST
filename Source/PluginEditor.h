/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "NumberScreen.h"
#include "Background.h"

//==============================================================================
/**
*/
class BitDosAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BitDosAudioProcessorEditor (BitDosAudioProcessor&);
    ~BitDosAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;    
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;

private:
    void mouseMove(const juce::MouseEvent&) override {}

    Background background;
    NumberScreen display;


    BitDosAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitDosAudioProcessorEditor)
};
