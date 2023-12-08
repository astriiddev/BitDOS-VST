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
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

LookAndFeel::~LookAndFeel()
{
}

const juce::Font LookAndFeel::getCustomFont()
{
    /* Importing binary of custom font */
    static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::seg_7_otf, BinaryData::seg_7_otfSize);
    return juce::Font(typeface);
}

juce::Typeface::Ptr LookAndFeel::getTypefaceForFont(const juce::Font&)
{
    /* Returning font typeface */
    return getCustomFont().getTypefacePtr();
}