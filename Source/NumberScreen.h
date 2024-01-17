/*
  ==============================================================================

    NumberScreen.h
    Created: 24 Sep 2023 1:08:06am
    Author:  _astriid_

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class NumberScreen  : public juce::Component,
                             juce::Timer
{
public:
    NumberScreen(BitDosAudioProcessor&);
    ~NumberScreen() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void volEdit(juce::Graphics&, int bit1, int bit2, float param, bool isEditing);

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    void handleBitClick(int bit);

private:
    void bitInit();
    void timerCallback() override;

    const juce::Font sansFont = juce::Font(juce::Font::getDefaultSansSerifFontName(),
                                           18.0f, juce::Font::plain);

    const juce::Font sevenSegFont = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::seg_7_otf, 
                                                                            BinaryData::seg_7_otfSize));

    uint8_t currentSample{ 0 };

    bool editingPreGain { false },
         editingPostGain{ false },
         editingBlend   { false };

    const int bitW{ 49 }, bitY{ 12 }, bitH{ 70 };

    int mouseOverBit{ -1 };
   
    typedef struct
    {
        const juce::Rectangle<int> rect;
        juce::Colour color;
    } BitScreen;

    const juce::Colour LED_RED = juce::Colour(0xFFBB0000),
                       LED_GRN = juce::Colour(0xFF00BB00),
                       LED_AMB = juce::Colour(0xFFD26200),
                       LED_WHT = juce::Colour(0xFFBBBBBB);

    BitScreen bits[8] = { {{ 352, bitY, bitW, bitH }, LED_GRN },
                          {{ 304, bitY, bitW, bitH }, LED_GRN },
                          {{ 255, bitY, bitW, bitH }, LED_GRN },
                          {{ 206, bitY, bitW, bitH }, LED_GRN },
                          {{ 157, bitY, bitW, bitH }, LED_GRN },
                          {{ 108, bitY, bitW, bitH }, LED_GRN },
                          {{  59, bitY, bitW, bitH }, LED_GRN },
                          {{  10, bitY, bitW, bitH }, LED_GRN } };

    const juce::Rectangle<int> screenNums { 10, bitY, 388, bitH };

    float lastMouseY = 0.0f;

    BitDosAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumberScreen)
};
