/*
 This file is part of the SimpleCompressor project.
 https://github.com/DanielRudrich/SimpleCompressor
 Copyright (c) 2019 Daniel Rudrich

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "GainReductionComputer.h"
using namespace dsp;

/**
 A simple compressor class, which wraps the GainReductionComputer in a JUCE audio processor. All parameter setter and getter are forwarded to the GainReductionComputer inside this class. Alternatively you can call getGainReductionComputer to get a reference to this member and set the parameters directly.
 */
class SimpleCompressor : private ProcessorBase
{
public:
    SimpleCompressor() {};

    // ======================================================================
    /**
     Sets the attack time of the compressor in seconds.
     */
    void setAttackTime (const float attackTimeInSeconds) { gainReductionComputer.setAttackTime (attackTimeInSeconds); };

    /**
     Sets the release time of the compressorin seconds
     */
    void setReleaseTime (const float releaseTimeInSeconds) { gainReductionComputer.setReleaseTime (releaseTimeInSeconds); };

    /**
     Sets the knee-width in decibels.
     */
    void setKnee (const float kneeInDecibels) { gainReductionComputer.setKnee (kneeInDecibels); };
    const float getKnee() { return gainReductionComputer.getKnee(); }

    /**
     Sets the threshold above which the compressor will start to compress the signal.
     */
    void setThreshold (const float thresholdInDecibels) { gainReductionComputer.setThreshold (thresholdInDecibels); };
    const float getThreshold() { return gainReductionComputer.getThreshold(); }

    /**
     Sets the make-up-gain of the compressor in decibels.
     */
    void setMakeUpGain (const float makeUpGainInDecibels) { gainReductionComputer.setMakeUpGain (makeUpGainInDecibels); };
    const float getMakeUpGain() { return gainReductionComputer.getMakeUpGain(); }

    /**
     Sets the ratio of input-output signal above threshold. Set to 1 for no compression, up to infinity for a brickwall limiter.
     */
    void setRatio (const float ratio) { gainReductionComputer.setRatio (ratio); };

    // ======================================================================
    /**
     Computes the static output levels for an array of input levels in decibels. Useful for visualization of the compressor's characteristic. Will contain make-up gain.
     */
    void getCharacteristic (float* inputLevelsInDecibels, float* destination, const int numSamples) { gainReductionComputer.getCharacteristic (inputLevelsInDecibels, destination, numSamples); }

    /**
     Computes the static output levels for a given input level in decibels. Useful for visualization of the compressor's characteristic. Will contain make-up gain.
     */
    float getCharacteristicSample (const float inputLevelInDecibels) { return gainReductionComputer.getCharacteristicSample (inputLevelInDecibels); };

    void prepare (const ProcessSpec& specs) override
    {
        gainReductionComputer.prepare (specs.sampleRate);
        sideChainBuffer.setSize (2, specs.maximumBlockSize);
    };

    void process (const ProcessContextReplacing<float>& context) override
    {
        auto outBlock = context.getOutputBlock();
        const int nChannels = static_cast<int> (outBlock.getNumChannels());
        const int numSamples = static_cast<int> (outBlock.getNumSamples());

        /** STEP 1: compute sidechain-signal */
        // copy the absolute values from the first input channel to the sideChainBuffer
        FloatVectorOperations::abs (sideChainBuffer.getWritePointer (0), outBlock.getChannelPointer (0), numSamples);

        // copy all other channels to the second channel of the sideChainBuffer and write the maximum of both channels to the first one
        for (int ch = 1; ch < nChannels; ++ch)
        {
            FloatVectorOperations::abs (sideChainBuffer.getWritePointer (1), outBlock.getChannelPointer (ch), numSamples);
            FloatVectorOperations::max (sideChainBuffer.getWritePointer (0), sideChainBuffer.getReadPointer (0), sideChainBuffer.getReadPointer (1), numSamples);
        }

        /** STEP 2: calculate gain reduction */
        gainReductionComputer.computeLinearGainFromSidechainSignal (sideChainBuffer.getReadPointer (0), sideChainBuffer.getWritePointer (1), numSamples);
        // gain-reduction is now in the second channel of our sideChainBuffer


        /** STEP 3: apply gain-reduction to all channels */
        for (int ch = 0; ch < nChannels; ++ch)
            FloatVectorOperations::multiply (outBlock.getChannelPointer (ch), sideChainBuffer.getReadPointer (1), numSamples);
    };

    void reset() override {};

    GainReductionComputer& getGainReductionComputer() { return gainReductionComputer; };

private:
    GainReductionComputer gainReductionComputer;
    AudioBuffer<float> sideChainBuffer;
};
