/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LookAheadCompressorAudioProcessorEditor::LookAheadCompressorAudioProcessorEditor (LookAheadCompressorAudioProcessor& p, AudioProcessorValueTreeState& params)
    : AudioProcessorEditor (&p), processor (p), parameters (params),
    cv (processor.getCompressor()),
    threshold (Slider::RotaryVerticalDrag, Slider::TextBoxBelow),
    knee (Slider::RotaryVerticalDrag, Slider::TextBoxBelow),
    attack (Slider::RotaryVerticalDrag, Slider::TextBoxBelow),
    release (Slider::RotaryVerticalDrag, Slider::TextBoxBelow),
    ratio (Slider::RotaryVerticalDrag, Slider::TextBoxBelow),
    makeUp (Slider::RotaryVerticalDrag, Slider::TextBoxBelow)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    addAndMakeVisible (threshold);
    thresholdAttachment.reset (new SliderAttachment (parameters, "threshold", threshold));
    threshold.setTextValueSuffix (" dB");

    addAndMakeVisible (knee);
    kneeAttachment.reset (new SliderAttachment (parameters, "knee", knee));
    knee.setTextValueSuffix (" dB");

    addAndMakeVisible (attack);
    attackAttachment.reset (new SliderAttachment (parameters, "attack", attack));
    attack.setTextValueSuffix (" ms");

    addAndMakeVisible (release);
    releaseAttachment.reset (new SliderAttachment (parameters, "release", release));
    release.setTextValueSuffix (" ms");

    addAndMakeVisible (ratio);
    ratioAttachment.reset (new SliderAttachment (parameters, "ratio", ratio));
    ratio.setTextValueSuffix (" : 1");

    addAndMakeVisible (makeUp);
    makeUpAttachment.reset (new SliderAttachment (parameters, "makeUp", makeUp));
    makeUp.setTextValueSuffix (" dB");

    addAndMakeVisible (lookAhead);
    lookAheadAttachment.reset (new ButtonAttachment (parameters, "lookAhead", lookAhead));
    lookAhead.setButtonText("look-ahead processing (5 ms)");

    addAndMakeVisible (cv);
    startTimer (50);
}

LookAheadCompressorAudioProcessorEditor::~LookAheadCompressorAudioProcessorEditor()
{
}

//==============================================================================
void LookAheadCompressorAudioProcessorEditor::paint (Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (20.0f);

    g.drawFittedText ("LookAheadCompressor", bounds.removeFromTop (25), Justification::centred, 1);

    auto labelRow = bounds.removeFromBottom (12);

    g.setFont (10.0f);
    g.drawFittedText ("Threshold", labelRow.removeFromLeft (60), 12, Justification::centred, 1);
    labelRow.removeFromLeft (8);
    g.drawFittedText ("Knee", labelRow.removeFromLeft (60), 12, Justification::centred, 1);
    labelRow.removeFromLeft (8);
    g.drawFittedText ("Attack", labelRow.removeFromLeft (60), 12, Justification::centred, 1);
    labelRow.removeFromLeft (8);
    g.drawFittedText ("Release", labelRow.removeFromLeft (60), 12, Justification::centred, 1);
    labelRow.removeFromLeft (8);
    g.drawFittedText ("Ratio", labelRow.removeFromLeft (60), 12, Justification::centred, 1);
    labelRow.removeFromLeft (8);
    g.drawFittedText ("MakeUp", labelRow.removeFromLeft (60), 12, Justification::centred, 1);

}

void LookAheadCompressorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop (22);
    bounds.removeFromBottom (12);

    auto row = bounds.removeFromBottom (80);

    threshold.setBounds (row.removeFromLeft (60));
    row.removeFromLeft (8);
    knee.setBounds (row.removeFromLeft (60));
    row.removeFromLeft (8);
    attack.setBounds (row.removeFromLeft (60));
    row.removeFromLeft (8);
    release.setBounds (row.removeFromLeft (60));
    row.removeFromLeft (8);
    ratio.setBounds (row.removeFromLeft (60));
    row.removeFromLeft (8);
    makeUp.setBounds (row.removeFromLeft (60));

    lookAhead.setBounds (bounds.removeFromBottom (20));
    cv.setBounds (bounds.reduced (10));
}


void LookAheadCompressorAudioProcessorEditor::timerCallback()
{
    if (processor.characteristicChanged.get())
    {
        processor.characteristicChanged = false;
        cv.updateCharacteristic();
    }

    cv.repaint();
}
