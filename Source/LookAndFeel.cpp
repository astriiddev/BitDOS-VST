/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 23 Sep 2023 7:12:37pm
    Author:  _astriid_

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LookAndFeel.h"

//==============================================================================
LookAndFeel::LookAndFeel()
{
    /* Importing binary of custom font */
    typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::seg_7_otf, BinaryData::seg_7_otfSize);
    sevenSegFont = juce::Font(typeface);
}

LookAndFeel::~LookAndFeel()
{
}

const juce::Font LookAndFeel::getCustomFont()
{
    return sevenSegFont;
}

juce::Typeface::Ptr LookAndFeel::getTypefaceForFont(const juce::Font&)
{
    return typeface;
}