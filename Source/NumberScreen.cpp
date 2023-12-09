/*
  ==============================================================================

    NumberScreen.cpp
    Created: 24 Sep 2023 1:08:06am
    Author:  _astriid_

  ==============================================================================
*/

#include <JuceHeader.h>
#include "NumberScreen.h"

//==============================================================================
NumberScreen::NumberScreen(BitDosAudioProcessor& p) : newLook(), audioProcessor(p)
{
    startTimer(60);

    bitInit();
}

NumberScreen::~NumberScreen()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void NumberScreen::paint (juce::Graphics& g)
{

    if (audioProcessor.getBypassState())
    {
        if (getMouseCursor() != juce::MouseCursor::NormalCursor)
            setMouseCursor(juce::MouseCursor::NormalCursor);

            return;
    }

    currentSample = audioProcessor.getBitSample();

    theFont = newLook.getCustomFont();
    theFont.setHeight(83);
    g.setFont(theFont);

    g.setColour(juce::Colour(0xad00102b));

    g.drawText("88888888", screenNums, juce::Justification::topLeft, false);

    if(audioProcessor.inBitMode())
    {
        for (int i = 7; i >= 0; i--)
        {
            juce::String bitScreen = juce::String((currentSample & (1 << i)) >> i);

            g.setColour(bits[i].color);
            g.drawText(bitScreen, bits[i].rect, juce::Justification::topLeft, false);

            g.setColour(juce::Colour(0x20d3d3d3));

            if (bits[i].rect.contains(getMouseXYRelative()))
            {
                setMouseCursor(juce::MouseCursor::PointingHandCursor);
                g.fillRect(bits[i].rect);
            }
        }
    }
    else
    {
        volEdit(g, 7, 6, (audioProcessor.getPreGain() * 127.5f), editingPreGain);
        volEdit(g, 4, 3, (audioProcessor.getPostGain() * 127.5f), editingPostGain);
        volEdit(g, 1, 0, (audioProcessor.getBlend()    * 255.0f), editingBlend);

        g.setColour(LED_WHT);

        g.setFont(sansFont);
        g.drawText("PRE GAIN",  juce::Rectangle<int>( 20, 100,  80, 20), juce::Justification::topLeft, false);
        g.drawText("POST GAIN", juce::Rectangle<int>(162, 100, 100, 20), juce::Justification::topLeft, false);
        g.drawText("BLEND",     juce::Rectangle<int>(322, 100,  60, 20), juce::Justification::topLeft, false);
    }
}

void NumberScreen::resized()
{
}

void NumberScreen::volEdit(juce::Graphics& g, int bit1, int bit2, float param, bool isEditing)
{
    uint8_t float2nib = (uint8_t)round(param);

    juce::String bitScreen = juce::String::toHexString((float2nib & ~0x0F) >> 4).toUpperCase();

    g.setColour(juce::Colour(0xFFBBBBBB));

    g.drawText(bitScreen, bits[bit1].rect, juce::Justification::topLeft, false);

    bitScreen = juce::String::toHexString(float2nib & ~0xF0).toUpperCase();

    g.drawText(bitScreen, bits[bit2].rect, juce::Justification::topLeft, false);

    if ((bits[bit1].rect.contains(getMouseXYRelative()) || bits[bit2].rect.contains(getMouseXYRelative())) ||
        isEditing)
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        g.setColour(juce::Colour(0x20d3d3d3));
        g.fillRect(bits[bit1].rect);
        g.fillRect(bits[bit2].rect);
    }
}

void NumberScreen::mouseDown(const juce::MouseEvent& e)
{
    const juce::Point<int> mouseDownPos = e.getMouseDownPosition();
    const int numClicks = e.getNumberOfClicks();

    if(audioProcessor.inBitMode())
    {
        for (int i = 7; i >= 0; i--)
        {
            if (bits[i].rect.contains(mouseDownPos))
            {
                if (!audioProcessor.getBypassState())
                    handleBitClick(i);
            }
        }
    }
    else
    {
        if (bits[7].rect.contains(mouseDownPos) ||
            bits[6].rect.contains(mouseDownPos))
        {
            editingPreGain = true;

            if (numClicks >= 2)
                audioProcessor.setPreGain(1.0f);
        }

        if (bits[4].rect.contains(mouseDownPos) ||
            bits[3].rect.contains(mouseDownPos))
        {
            editingPostGain = true;

            if (numClicks >= 2)
                audioProcessor.setPostGain(1.0f);
        }

        if (bits[1].rect.contains(mouseDownPos) ||
            bits[0].rect.contains(mouseDownPos))
        {
            editingBlend = true;

            if (numClicks >= 2)
                audioProcessor.setBlend(1.0f);
        }
    }
}

void NumberScreen::mouseMove(const juce::MouseEvent& e)
{
    const juce::Point<int> mousePos = e.getPosition();
    mouseOverBit = -1;

    for (int i = 7; i >= 0; i--)
    {

        if (bits[i].rect.contains(mousePos))
        {
            mouseOverBit = i;
        }
    }

    if (!audioProcessor.inBitMode() && getMouseCursor() != juce::MouseCursor::NormalCursor)
        if(mouseOverBit == 5 || mouseOverBit == 2)
            setMouseCursor(juce::MouseCursor::NormalCursor);
}

void NumberScreen::mouseDrag(const juce::MouseEvent& e)
{
    float gainChange = 0.0f,
        gainChangeScale = -2000.0f;

    static float lastMouseY = 0.0f;

    if (audioProcessor.inBitMode()) return;

    const float mouseDragY = (float)e.getDistanceFromDragStartY();

    if (lastMouseY > abs(mouseDragY))
        gainChangeScale = 2000.0f;

    if (editingPreGain)
    {
        gainChange = mouseDragY / gainChangeScale;

        audioProcessor.setPreGain(audioProcessor.getPreGain() + gainChange);
    }

    if (editingPostGain)
    {
        gainChange = mouseDragY / gainChangeScale;

        audioProcessor.setPostGain(audioProcessor.getPostGain() + gainChange);
    }

    if (editingBlend)
    {
        gainChange = mouseDragY / gainChangeScale;

        audioProcessor.setBlend(audioProcessor.getBlend() + gainChange);
    }

    lastMouseY = abs(mouseDragY);
}

void NumberScreen::mouseUp(const juce::MouseEvent&)
{
    if (editingPreGain)  editingPreGain  = false;
    if (editingPostGain) editingPostGain = false;
    if (editingBlend)    editingBlend    = false;
}

void NumberScreen::mouseExit(const juce::MouseEvent&)
{
    for (int i = 7; i >= 0; i--)
    {
        repaint(bits[i].rect);
    }

    mouseOverBit = -1;
}

void NumberScreen::handleBitClick(int bit)
{
    switch (audioProcessor.getBitSet(bit))
    {
    case (NORMAL_BIT):
        audioProcessor.setBitInvert(bit);

        bits[bit].color = LED_AMB;
        audioProcessor.setBitSet(INVERT_BIT, bit);
        break;

    case (INVERT_BIT):

        audioProcessor.setBitZeroed(bit);

        bits[bit].color = LED_RED;
        audioProcessor.setBitSet(HARD_0_BIT, bit);
        break;

    case (HARD_0_BIT):

        audioProcessor.resetBit(bit);

        bits[bit].color = LED_GRN;
        audioProcessor.setBitSet(NORMAL_BIT, bit);
        break;
    }

    repaint(bits[bit].rect);
}

void NumberScreen::bitInit()
{
    for (int i = 7; i >= 0; i--)
    {

        switch (audioProcessor.getBitSet(i))
        {
        case (NORMAL_BIT):

            bits[i].color = LED_GRN;
            break;

        case (INVERT_BIT):

            bits[i].color = LED_AMB;
            break;

        case (HARD_0_BIT):

            bits[i].color = LED_RED;
            break;
        }
    }
}

void NumberScreen::timerCallback()
{
    static bool lastMode = false;
    static juce::Point<int> lastMousePos = { 0, 0 };
    static float lastSamp = 0.0f;
    
    if (audioProcessor.getBypassState()) return;

    const juce::Point<int> mousePos = getMouseXYRelative();
    const float currRealSamp = audioProcessor.getCurrentSample();
    const bool currBitMode = audioProcessor.inBitMode();

    if (lastMode == currBitMode && lastSamp == currRealSamp)
        if (!(lastMousePos != mousePos && mouseOverBit > -1)) return;

    for (int i = 7; i >= 0; i--)
        repaint(bits[i].rect);

    lastMode = currBitMode;
    lastMousePos = mousePos;
    lastSamp = currRealSamp;
}
