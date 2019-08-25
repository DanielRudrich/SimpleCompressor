/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleCompressorAudioProcessor::SimpleCompressorAudioProcessor() :
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
parameters (*this, nullptr, Identifier ("SimpleCompressorState"), createParameterLayout())
{
    parameters.addParameterListener ("threshold", this);
    parameters.addParameterListener ("knee", this);
    parameters.addParameterListener ("attack", this);
    parameters.addParameterListener ("release", this);
    parameters.addParameterListener ("ratio", this);
    parameters.addParameterListener ("makeUp", this);

    compressor.setThreshold (*parameters.getRawParameterValue ("threshold"));
    compressor.setKnee (*parameters.getRawParameterValue ("knee"));
    compressor.setAttackTime (*parameters.getRawParameterValue ("attack") / 1000);
    compressor.setReleaseTime (*parameters.getRawParameterValue ("release") / 1000);
    compressor.setMakeUpGain (*parameters.getRawParameterValue ("makeUp"));
    const float ratio = *parameters.getRawParameterValue ("ratio");
    if (ratio > 15.9f)
        compressor.setRatio (std::numeric_limits<float>::infinity());
    else
        compressor.setRatio (ratio);
}

SimpleCompressorAudioProcessor::~SimpleCompressorAudioProcessor()
{
}

//==============================================================================
const String SimpleCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const String SimpleCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleCompressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SimpleCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    compressor.prepare ({sampleRate, static_cast<uint32> (samplesPerBlock), 2});
}

void SimpleCompressorAudioProcessor::releaseResources()
{
    compressor.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleCompressorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    const int numSamples = buffer.getNumSamples();

    // clear not needed output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    
    AudioBlock<float> ab (buffer);
    ProcessContextReplacing<float> context (ab);
    compressor.process (context);
}

//==============================================================================
bool SimpleCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SimpleCompressorAudioProcessor::createEditor()
{
    return new SimpleCompressorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void SimpleCompressorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleCompressorAudioProcessor();
}

// =============================================================================
AudioProcessorValueTreeState::ParameterLayout SimpleCompressorAudioProcessor::createParameterLayout()
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

    return { params.begin(), params.end() };
}

void SimpleCompressorAudioProcessor::parameterChanged (const String& parameterID, float newValue)
{
    if (parameterID == "threshold")
    {
        compressor.setThreshold (newValue);
        characteristicChanged = true;
    }
    else if (parameterID == "knee")
    {
        compressor.setKnee (newValue);
        characteristicChanged = true;
    }
    else if (parameterID == "attack")
        compressor.setAttackTime (newValue / 1000);
    else if (parameterID == "release")
        compressor.setReleaseTime (newValue / 1000);
    else if (parameterID == "ratio")
    {
        if (newValue > 15.9f)
            compressor.setRatio (std::numeric_limits<float>::infinity());
        else
            compressor.setRatio (newValue);
        characteristicChanged = true;
    }
    else if (parameterID == "makeUp")
    {
        compressor.setMakeUpGain (newValue);
        characteristicChanged = true;
    }
    else
        jassertfalse;
}
