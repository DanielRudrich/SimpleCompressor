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
#include <vector>

class LookAheadGainReduction
{
public:

    LookAheadGainReduction() : sampleRate (0.0) {}
    ~LookAheadGainReduction() {}

    void setDelayTime (float delayTimeInSeconds);

    const int getDelayInSamples() { return delayInSamples; }

    void prepare (const double sampleRate, const int blockSize);

    /**
     Writes gain-reduction samples into the delay-line. Make sure you call process() afterwards, and read the same amount of samples with the readSamples method.
     */
    void pushSamples (const float* src, const int numSamples);

    /**
     Processes the data within the delay line, i.e. fades-in high gain-reduction, in order to reduce distortions.
     */
    void process();

    /**
     Reads smoothed gain-reduction samples back to the destination. Make sure you read as many samples as you've pushed before!
     */
    void readSamples (float* dest, const int numSamples);


private:
    inline void getProcessPositions (int startIndex, int numSamples, int& blockSize1, int& blockSize2);

    inline void getWritePositions (int numSamples, int& startIndex, int& blockSize1, int& blockSize2);

    inline void getReadPositions (int numSamples, int& startIndex, int& blockSize1, int& blockSize2);


private:
    //==============================================================================
    double sampleRate;
    int blockSize;

    float delay;
    int delayInSamples = 0;
    int writePosition = 0;
    int lastPushedSamples = 0;
    std::vector<float> buffer;
};
