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


#include "LookAheadGainReduction.h"
#include <cmath>

void LookAheadGainReduction::setDelayTime (float delayTimeInSeconds)
{
    if (delayTimeInSeconds <= 0.0f)
        delay = 0.0f;
    else
        delay = delayTimeInSeconds;

    if (sampleRate != 0.0)
        prepare (sampleRate, blockSize);
}

void LookAheadGainReduction::prepare (const double newSampleRate, const int newBlockSize)
{
    sampleRate = newSampleRate;
    blockSize = newBlockSize;

    delayInSamples = static_cast<int> (delay * sampleRate);

    buffer.resize (blockSize + delayInSamples);
    std::fill (buffer.begin(), buffer.end(), 0.0f);
    writePosition = 0;
}

void LookAheadGainReduction::pushSamples (const float* src, const int numSamples)
{
    int startIndex, blockSize1, blockSize2;

    // write in delay line
    getWritePositions (numSamples, startIndex, blockSize1, blockSize2);

    for (int i = 0; i < blockSize1; ++i)
        buffer[startIndex + i] = src[i];

    if (blockSize2 > 0)
        for (int i = 0; i < blockSize2; ++i)
            buffer[i] = src[blockSize1 + i];

    writePosition += numSamples;
    writePosition = writePosition % buffer.size();

    lastPushedSamples = numSamples;
}

void LookAheadGainReduction::process()
{
    float nextGainReductionValue = 0.0f;
    float step = 0.0f;

    // last pushed samples
    int startIndex = writePosition - 1;
    if (startIndex < 0)
        startIndex += static_cast<int> (buffer.size());

    int size1, size2;

    getProcessPositions (startIndex, lastPushedSamples, size1, size2);

    for (int i = 0; i < size1; ++i)
    {
        float smpl = buffer[startIndex];

        if (smpl > nextGainReductionValue)
        {
            buffer[startIndex] = nextGainReductionValue;
            nextGainReductionValue += step;
        }
        else
        {
            step = - smpl / delayInSamples;
            nextGainReductionValue = smpl + step;
        }
        --startIndex;
    }
    if (size2 > 0)
    {
        startIndex = static_cast<int> (buffer.size()) - 1;
        for (int i = 0; i < size2; ++i)
        {
            float smpl = buffer[startIndex];
            if (smpl > nextGainReductionValue)
            {
                buffer[startIndex] = nextGainReductionValue;
                nextGainReductionValue += step;
            }
            else
            {
                step = - smpl / delayInSamples;
                nextGainReductionValue = smpl + step;
            }
            --startIndex;
        }
    }

    if (startIndex < 0)
        startIndex = static_cast<int> (buffer.size()) - 1;

    getProcessPositions (startIndex, delayInSamples, size1, size2);
    bool breakWasUsed = false;

    for (int i = 0; i < size1; ++i)
    {
        float smpl = buffer[startIndex];
        if (smpl > nextGainReductionValue)
        {
            buffer[startIndex] = nextGainReductionValue;
            nextGainReductionValue += step;
        }
        else
        {
            breakWasUsed = true;
            break;
        }
        --startIndex;
    }
    if (! breakWasUsed && size2 > 0)
    {
        startIndex = static_cast<int> (buffer.size()) - 1;
        for (int i = 0; i < size2; ++i)
        {
            float smpl = buffer[startIndex];
            if (smpl > nextGainReductionValue)
            {
                buffer[startIndex] = nextGainReductionValue;
                nextGainReductionValue += step;
            }
            else
            {
                break;
            }
            --startIndex;
        }
    }
}


void LookAheadGainReduction::readSamples (float* dest, int numSamples)
{
    int startIndex, blockSize1, blockSize2;

    // read from delay line
    getReadPositions (numSamples, startIndex, blockSize1, blockSize2);

    for (int i = 0; i < blockSize1; ++i)
        dest[i] = buffer[startIndex + i];

    if (blockSize2 > 0)
        for (int i = 0; i < blockSize2; ++i)
            dest[blockSize1 + i] = buffer[i];
}


inline void LookAheadGainReduction::getProcessPositions (int startIndex, int numSamples, int& blockSize1, int& blockSize2)
{
    if (numSamples <= 0)
    {
        blockSize1 = 0;
        blockSize2 = 0;
    }
    else
    {
        blockSize1 = std::min (startIndex + 1, numSamples);
        numSamples -= blockSize1;
        blockSize2 = numSamples <= 0 ? 0 : numSamples;
    }
}

inline void LookAheadGainReduction::getWritePositions (int numSamples, int& startIndex, int& blockSize1, int& blockSize2)
{
    const int L = static_cast<int> (buffer.size());
    int pos = writePosition;

    if (pos < 0)
        pos = pos + L;
    pos = pos % L;

    if (numSamples <= 0)
    {
        startIndex = 0;
        blockSize1 = 0;
        blockSize2 = 0;
    }
    else
    {
        startIndex = pos;
        blockSize1 = std::min (L - pos, numSamples);
        numSamples -= blockSize1;
        blockSize2 = numSamples <= 0 ? 0 : numSamples;
    }
}

inline void LookAheadGainReduction::getReadPositions (int numSamples, int& startIndex, int& blockSize1, int& blockSize2)
{
    const int L = static_cast<int> (buffer.size());
    int pos = writePosition - lastPushedSamples - delayInSamples;

    if (pos < 0)
        pos = pos + L;
    pos = pos % L;

    if (numSamples <= 0)
    {
        startIndex = 0;
        blockSize1 = 0;
        blockSize2 = 0;
    }
    else
    {
        startIndex = pos;
        blockSize1 = std::min (L - pos, numSamples);
        numSamples -= blockSize1;
        blockSize2 = numSamples <= 0 ? 0 : numSamples;
    }
}
