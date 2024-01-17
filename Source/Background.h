/*
  ==============================================================================

    Background.h
    Created: 24 Sep 2023 12:02:56pm
    Author:  _astriid_

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class Background  : public juce::Component,
                    public juce::Button::Listener
{
public:
    Background(BitDosAudioProcessor&);
    ~Background() override;

    void paintOverChildren(juce::Graphics&) override;

    void resized() override;

private:
    void buttonClicked(juce::Button* button) override;
    void mouseMove(const juce::MouseEvent&) override {}

    void initImgButton(juce::ImageButton*, const float up, const float down, const void* imgData, const int dataSize);

    juce::ImageComponent bgImg;

    juce::ImageButton onBtn, fpBtn, bdgBtn;

    const juce::Rectangle<int> onLED{ 122, 94, 23, 10 };

    BitDosAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Background)
};
