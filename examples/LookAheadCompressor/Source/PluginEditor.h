/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../../thirdparty/CompressorVisualizer.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class LookAheadCompressorAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    LookAheadCompressorAudioProcessorEditor (LookAheadCompressorAudioProcessor&, AudioProcessorValueTreeState&);
    ~LookAheadCompressorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LookAheadCompressorAudioProcessor& processor;
    AudioProcessorValueTreeState& parameters;

    CompressorVisualizer cv;

    Slider threshold, knee, attack, release, ratio, makeUp;
    std::unique_ptr<SliderAttachment> thresholdAttachment, kneeAttachment, attackAttachment,
        releaseAttachment, ratioAttachment, makeUpAttachment;

    ToggleButton lookAhead;
    std::unique_ptr<ButtonAttachment> lookAheadAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAheadCompressorAudioProcessorEditor)
};
