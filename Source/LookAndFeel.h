/*
  ==============================================================================

    LookAndFeel.h
    Created: 23 Sep 2023 7:12:37pm
    Author:  _astriid_

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    ~LookAndFeel() override;

    /* Custom global font */
    const juce::Font getCustomFont();
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel)
};
