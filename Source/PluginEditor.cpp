/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BitDosAudioProcessorEditor::BitDosAudioProcessorEditor (BitDosAudioProcessor& p)
    : AudioProcessorEditor (&p), background(p), display(p), audioProcessor (p)
{
    addAndMakeVisible(background);
    addAndMakeVisible(display);

    addMouseListener(&display, false);

    setSize(720, 526);

    setWantsKeyboardFocus(false);
}

BitDosAudioProcessorEditor::~BitDosAudioProcessorEditor()
{
}

//==============================================================================
void BitDosAudioProcessorEditor::paint (juce::Graphics&)
{
}

void BitDosAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{   
    if(audioProcessor.getBypassState())
    {
        g.setColour(juce::Colour(0x40000000));
        g.fillAll();
    }
}

void BitDosAudioProcessorEditor::resized()
{
    background.setBounds(0, 0, 720, 526);
    display.setBounds(88, 307, 412, 130);
}