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
    const juce::Image bg = juce::ImageCache::getFromMemory(BinaryData::bitdos_gui_png, BinaryData::bitdos_gui_pngSize);

    addAndMakeVisible(bgImg);

    if (!bg.isNull())
        bgImg.setImage(bg, juce::RectanglePlacement::stretchToFit);
    else
        jassert(!bg.isNull());

    initImgButton(&bdgBtn,1.0f, 1.0f, BinaryData::bitdos_badge_png, BinaryData::bitdos_badge_pngSize);
    initImgButton(&fpBtn, 0.0f, 1.0f, BinaryData::floppy_btn_png, BinaryData::floppy_btn_pngSize);
    initImgButton(&onBtn, 1.0f, 0.0f, BinaryData::bitdos_on_png, BinaryData::bitdos_on_pngSize);

    bgImg.setBufferedToImage(true);

    onBtn.setClickingTogglesState(true);
    setRepaintsOnMouseActivity(false);
}

Background::~Background()
{
}

void Background::paintOverChildren(juce::Graphics& g)
{
    if (audioProcessor.getBypassState())
    {
        onBtn.setToggleState(true, juce::NotificationType::dontSendNotification);
        return;
    }

    if (audioProcessor.inSignedMode())
        g.setColour(juce::Colour(0x80DD0000));
    else
        g.setColour(juce::Colour(0x5000DD00));

    if (fpBtn.getState() != juce::Button::buttonDown)
        g.fillRect(onLED);
}

void Background::resized()
{
    bgImg.setBounds(0, 0, 720, 526);
    onBtn.setBounds(485, 278, 147, 144);
    fpBtn.setBounds(350, 87, 65, 23);
    bdgBtn.setBounds(550, 70, 100, 100);
}

void Background::buttonClicked(juce::Button* button)
{
    if (button == &onBtn)
    {
        audioProcessor.setBypassState();
        repaint();
    }

    if (audioProcessor.getBypassState()) return;
    
    if (button == &fpBtn)
    {
        audioProcessor.setSignedMode();
        repaint(onLED);
    }

    if (button == &bdgBtn)
    {
        audioProcessor.setBitMode();
        repaint(100, 375, 400, 100);
    }
}

void Background::initImgButton(juce::ImageButton* b, const float up, const float down, const void* imgData, const int dataSize)
{
    juce::Image img;

    jassert(b != nullptr);
    jassert(imgData != nullptr);

    img = juce::ImageCache::getFromMemory(imgData, dataSize);

    if (img.isNull())
    {
        jassert(!img.isNull());
        return;
    }

    addAndMakeVisible(*b);

    b->addListener(this);
    b->setRepaintsOnMouseActivity(false);
    b->setMouseCursor(juce::MouseCursor::PointingHandCursor);

    b->setImages(true, true, true,
                 img, up, juce::Colours::transparentWhite,
                 img, up, juce::Colours::transparentWhite,
                 img, down, juce::Colours::transparentWhite);
}
