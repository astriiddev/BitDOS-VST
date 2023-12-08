/*
  ==============================================================================

    Background.cpp
    Created: 24 Sep 2023 12:02:56pm
    Author:  _astriid_

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Background.h"

//==============================================================================
Background::Background(BitDosAudioProcessor& p) : audioProcessor(p)
{
    addAndMakeVisible(bgImg);

    addAndMakeVisible(onBtn);
    addAndMakeVisible(fpBtn);
    addAndMakeVisible(badge);

    onBtn.setClickingTogglesState(true);

    auto bg = juce::ImageCache::getFromMemory(BinaryData::bitdos_gui_png, 
                                              BinaryData::bitdos_gui_pngSize);
    if (!bg.isNull())
        bgImg.setImage(bg, juce::RectanglePlacement::stretchToFit);
    else
        jassert(!bg.isNull());

    auto on = juce::ImageCache::getFromMemory(BinaryData::bitdos_on_png, 
                                              BinaryData::bitdos_on_pngSize);
    if (!on.isNull())
        onBtn.setImages(true, true, true,
                        on, 1.0, juce::Colour(0), 
                        on, 1.0, juce::Colour(0), 
                        on, 0.0, juce::Colour(0),
                        0.0f);
    else
        jassert(!on.isNull());

    auto fp = juce::ImageCache::getFromMemory(BinaryData::floppy_btn_png, 
                                              BinaryData::floppy_btn_pngSize);
    if (!fp.isNull())
        fpBtn.setImages(true, true, true,
                        fp, 0.0, juce::Colour(0),
                        fp, 0.0, juce::Colour(0),
                        fp, 1.0, juce::Colour(0),
                        0.0f);
    else
        jassert(!fp.isNull());

    onBtn.addListener(this);
    fpBtn.addListener(this);
    badge.addListener(this);

    onBtn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    fpBtn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    badge.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    bgImg.setBufferedToImage(true);
    onBtn.setBufferedToImage(true);
    fpBtn.setBufferedToImage(true);

    badge.setAlpha(0.0f);
}

Background::~Background()
{
}

void Background::paintOverChildren(juce::Graphics& g)
{
    if (!audioProcessor.getBypassState())
    {
        if (audioProcessor.inSignedMode())
            g.setColour(juce::Colour(0x80DD0000));
        else
            g.setColour(juce::Colour(0x5000DD00));

        if (fpBtn.getState() != juce::Button::buttonDown)
            g.fillRect(onLED);
    }
    else
        onBtn.setToggleState(true, juce::NotificationType::dontSendNotification);
}

void Background::resized()
{
    bgImg.setBounds(0, 0, 720, 526);
    onBtn.setBounds(485, 278, 147, 144);
    fpBtn.setBounds(350, 87, 65, 23);
    badge.setBounds(540, 65, 110, 110);
}

void Background::buttonClicked(juce::Button* button)
{
    if (button == &onBtn)
    {
        audioProcessor.setBypassState();
        repaint();
    }

    if(!audioProcessor.getBypassState())
    {
        if (button == &fpBtn)
            audioProcessor.setSignedMode();

        if (button == &badge)
        {
            audioProcessor.setBitMode();
            repaint(100, 375, 400, 100);
        }
    }
}

void Background::buttonStateChanged(juce::Button* button)
{
    if(button == &fpBtn)
        repaint(onLED);
}
