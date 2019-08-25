/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../../src/GainReductionComputer.h"
#include "../../../src/LookAheadGainReduction.h"
#include "../../thirdparty/Delay.h"

//==============================================================================
/**
*/
class LookAheadCompressorAudioProcessor  : public AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    LookAheadCompressorAudioProcessor();
    ~LookAheadCompressorAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void parameterChanged (const String& parameterID, float newValue) override;

    GainReductionComputer& getCompressor() { return gainReductionComputer; };

    Atomic<bool> characteristicChanged = true;
private:
    AudioProcessorValueTreeState parameters;

    Delay delay;
    GainReductionComputer gainReductionComputer;
    LookAheadGainReduction lookAheadFadeIn;
    AudioBuffer<float> sideChainBuffer;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAheadCompressorAudioProcessor)
};
