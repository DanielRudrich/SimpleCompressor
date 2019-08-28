 /*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */


#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

using namespace dsp;
class Delay : private ProcessorBase
{
public:

    Delay()
    {
    }
    ~Delay() {}

    void setDelayTime (float delayTimeInSeconds)
    {
        if (delayTimeInSeconds <= 0.0f)
        {
            delay = 0.0f;
            bypassed = true;
        }
        else
        {
            delay = delayTimeInSeconds;
            bypassed = false;
        }

        prepare (spec);
    }

    const int getDelayInSamples()
    {
        return bypassed ? 0 : delayInSamples;
    }

    void prepare (const ProcessSpec& specs) override
    {
        spec = specs;

        delayInSamples = static_cast<int> (delay * specs.sampleRate);

        buffer.setSize (specs.numChannels, specs.maximumBlockSize + delayInSamples);
        buffer.clear();
        writePosition = 0;
    }

    void process (const ProcessContextReplacing<float>& context) override
    {
        ScopedNoDenormals noDenormals;

        if (! bypassed)
        {
            auto abIn = context.getInputBlock();
            auto abOut = context.getOutputBlock();
            auto L = static_cast<int> (abIn.getNumSamples());
            auto nCh = jmin((int) spec.numChannels, (int) abIn.getNumChannels());

            int startIndex, blockSize1, blockSize2;


            // write in delay line
            getReadWritePositions(false, (int) L, startIndex, blockSize1, blockSize2);

            for (int ch = 0; ch < nCh; ch++)
                buffer.copyFrom(ch, startIndex, abIn.getChannelPointer (ch), blockSize1);

            if (blockSize2 > 0)
                for (int ch = 0; ch < nCh; ch++)
                    buffer.copyFrom(ch, 0, abIn.getChannelPointer (ch) + blockSize1, blockSize2);


            // read from delay line
            getReadWritePositions(true, (int) L, startIndex, blockSize1, blockSize2);

            for (int ch = 0; ch < nCh; ch++)
                FloatVectorOperations::copy (abOut.getChannelPointer(ch), buffer.getReadPointer(ch) + startIndex, blockSize1);

            if (blockSize2 > 0)
                for (int ch = 0; ch < nCh; ch++)
                    FloatVectorOperations::copy (abOut.getChannelPointer(ch) + blockSize1, buffer.getReadPointer (ch), blockSize2);


            writePosition += L;
            writePosition = writePosition % buffer.getNumSamples();
        }
    }

    void reset() override
    {

    }

    void getReadWritePositions (bool read, int numSamples, int& startIndex, int& blockSize1, int& blockSize2)
    {
        const int L = buffer.getNumSamples();
        int pos = writePosition;
        if (read)
        {
            pos = writePosition - delayInSamples;
        }
        if (pos < 0)
            pos = pos + L;
        pos = pos % L;

        jassert(pos >= 0 && pos < L);

        if (numSamples <= 0)
        {
            startIndex = 0;
            blockSize1 = 0;
            blockSize2 = 0;
        }
        else
        {
            startIndex = pos;
            blockSize1 = jmin (L - pos, numSamples);
            numSamples -= blockSize1;
            blockSize2 = numSamples <= 0 ? 0 : numSamples;
        }
    }

private:
    //==============================================================================
    ProcessSpec spec = {-1, 0, 0};
    float delay;
    int delayInSamples = 0;
    bool bypassed = false;
    int writePosition = 0;
    AudioBuffer<float> buffer;
};
