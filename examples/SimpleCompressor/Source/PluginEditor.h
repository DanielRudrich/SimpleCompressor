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
class SimpleCompressorAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    SimpleCompressorAudioProcessorEditor (SimpleCompressorAudioProcessor&, AudioProcessorValueTreeState&);
    ~SimpleCompressorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleCompressorAudioProcessor& processor;
    AudioProcessorValueTreeState& parameters;

    CompressorVisualizer cv;

    Slider threshold, knee, attack, release, ratio, makeUp;
    std::unique_ptr<SliderAttachment> thresholdAttachment, kneeAttachment, attackAttachment,
        releaseAttachment, ratioAttachment, makeUpAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleCompressorAudioProcessorEditor)
};
