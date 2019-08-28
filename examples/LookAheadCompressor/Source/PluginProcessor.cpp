/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LookAheadCompressorAudioProcessor::LookAheadCompressorAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
parameters (*this, nullptr, Identifier ("LookAheadCompressorState"), createParameterLayout())
{
    parameters.addParameterListener ("threshold", this);
    parameters.addParameterListener ("knee", this);
    parameters.addParameterListener ("attack", this);
    parameters.addParameterListener ("release", this);
    parameters.addParameterListener ("ratio", this);
    parameters.addParameterListener ("makeUp", this);

    gainReductionComputer.setThreshold (*parameters.getRawParameterValue ("threshold"));
    gainReductionComputer.setKnee (*parameters.getRawParameterValue ("knee"));
    gainReductionComputer.setAttackTime (*parameters.getRawParameterValue ("attack") / 1000);
    gainReductionComputer.setReleaseTime (*parameters.getRawParameterValue ("release") / 1000);
    gainReductionComputer.setMakeUpGain (*parameters.getRawParameterValue ("makeUp"));
    const float ratio = *parameters.getRawParameterValue ("ratio");
    if (ratio > 15.9f)
        gainReductionComputer.setRatio (std::numeric_limits<float>::infinity());
    else
        gainReductionComputer.setRatio (ratio);

    delay.setDelayTime (0.005f);
    lookAheadFadeIn.setDelayTime (0.005f);
}

LookAheadCompressorAudioProcessor::~LookAheadCompressorAudioProcessor()
{
}

//==============================================================================
const String LookAheadCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LookAheadCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LookAheadCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LookAheadCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LookAheadCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LookAheadCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LookAheadCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LookAheadCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const String LookAheadCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void LookAheadCompressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void LookAheadCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    gainReductionComputer.prepare (sampleRate);
    lookAheadFadeIn.prepare (sampleRate, samplesPerBlock);
    delay.prepare ({sampleRate, static_cast<uint32> (samplesPerBlock), 2});

    sideChainBuffer.setSize (2, samplesPerBlock);

    if (*parameters.getRawParameterValue ("lookAhead") > 0.5f)
        setLatencySamples (static_cast<int> (0.005 * sampleRate));
    else
        setLatencySamples (0);
}

void LookAheadCompressorAudioProcessor::releaseResources()
{
    gainReductionComputer.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LookAheadCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LookAheadCompressorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    const bool useLookAhead = *parameters.getRawParameterValue ("lookAhead") > 0.5f;
    const int numSamples = buffer.getNumSamples();

    // clear not needed output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    /** STEP 1: compute sidechain-signal */
    // copy the absolute values from the first input channel to the sideChainBuffer
    FloatVectorOperations::abs (sideChainBuffer.getWritePointer (0), buffer.getReadPointer (0), numSamples);

    // copy all other channels to the second channel of the sideChainBuffer and write the maximum of both channels to the first one
    for (int ch = 1; ch < totalNumInputChannels; ++ch)
    {
        FloatVectorOperations::abs (sideChainBuffer.getWritePointer (1), buffer.getReadPointer (ch), numSamples);
        FloatVectorOperations::max (sideChainBuffer.getWritePointer (0), sideChainBuffer.getReadPointer (0), sideChainBuffer.getReadPointer (1), numSamples);
    }

    /** STEP 2: calculate gain reduction, which one depends on lookAhead */
    if (useLookAhead)
        gainReductionComputer.computeGainInDecibelsFromSidechainSignal (sideChainBuffer.getReadPointer (0), sideChainBuffer.getWritePointer (1), numSamples);
    else
    {
        gainReductionComputer.computeLinearGainFromSidechainSignal (sideChainBuffer.getReadPointer (0), sideChainBuffer.getWritePointer (1), numSamples);
    }
    // gain-reduction is now in the second channel of our sideChainBuffer


    /** STEP 3: fade-in gain reduction if look-ahead is enabled */
    if (useLookAhead)
    {
        // delay audio signal
        AudioBlock<float> ab (buffer);
        ProcessContextReplacing<float> context (ab);
        delay.process (context);

        // fade in gain reduction
        lookAheadFadeIn.pushSamples (sideChainBuffer.getReadPointer (1), numSamples);
        lookAheadFadeIn.process();
        lookAheadFadeIn.readSamples (sideChainBuffer.getWritePointer(1), numSamples);

        // add make-up and convert to linear gain
        const float makeUpGainInDecibels = gainReductionComputer.getMakeUpGain();
        for (int i = 0; i < numSamples; ++i)
            sideChainBuffer.setSample (1, i, Decibels::decibelsToGain (sideChainBuffer.getSample (1, i) + makeUpGainInDecibels));
    }


    /** STEP 4: apply gain-reduction to all channels */
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        FloatVectorOperations::multiply (buffer.getWritePointer (ch), sideChainBuffer.getReadPointer (1), numSamples);

}

//==============================================================================
bool LookAheadCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* LookAheadCompressorAudioProcessor::createEditor()
{
    return new LookAheadCompressorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void LookAheadCompressorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LookAheadCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LookAheadCompressorAudioProcessor();
}

// =============================================================================
AudioProcessorValueTreeState::ParameterLayout LookAheadCompressorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat> ("threshold", "Threshold", NormalisableRange<float> (-50.0f, 10.0f, 0.1f), -10.0f, "dB"));
    params.push_back (std::make_unique<AudioParameterFloat> ("knee", "Knee", NormalisableRange<float> (0.0f, 30.0f, 0.1f), 0.0f, "dB"));
    params.push_back (std::make_unique<AudioParameterFloat> ("attack", "Attack Time", NormalisableRange<float> (0.0f, 100.0f, 0.1f), 30.0f, "ms"));
    params.push_back (std::make_unique<AudioParameterFloat> ("release", "Release Time", NormalisableRange<float> (0.0f, 800.0f, 0.1f), 150.0f, "ms"));
    params.push_back (std::make_unique<AudioParameterFloat> ("ratio", "Ratio", NormalisableRange<float> (1.0f, 16.0f, 0.1f), 30.0f, " : 1",
                                                             AudioProcessorParameter::genericParameter,
                                                             [](float value, int maximumStringLength) { if (value > 15.9f) return String ("inf"); return String (value, 2);}));
    params.push_back (std::make_unique<AudioParameterFloat> ("makeUp", "MakeUp Gain", NormalisableRange<float> (-10.0f, 20.0f, 0.1f), 0.0f, "dB"));
    params.push_back (std::make_unique<AudioParameterBool> ("lookAhead", "Look-Ahead", false));

    return { params.begin(), params.end() };
}

void LookAheadCompressorAudioProcessor::parameterChanged (const String& parameterID, float newValue)
{
    if (parameterID == "threshold")
    {
        gainReductionComputer.setThreshold (newValue);
        characteristicChanged = true;
    }
    else if (parameterID == "knee")
    {
        gainReductionComputer.setKnee (newValue);
        characteristicChanged = true;
    }
    else if (parameterID == "attack")
        gainReductionComputer.setAttackTime (newValue / 1000);
    else if (parameterID == "release")
        gainReductionComputer.setReleaseTime (newValue / 1000);
    else if (parameterID == "ratio")
    {
        if (newValue > 15.9f)
            gainReductionComputer.setRatio (std::numeric_limits<float>::infinity());
        else
            gainReductionComputer.setRatio (newValue);

        characteristicChanged = true;
    }
    else if (parameterID == "makeUp")
    {
        gainReductionComputer.setMakeUpGain (newValue);
        characteristicChanged = true;
    }
    else
        jassertfalse;
}
