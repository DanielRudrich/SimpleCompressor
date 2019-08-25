# Documentation: Included classes

## Preface: About the compressor implementation
There are many different ways to realize a compressor. There are feed-forward and feed-back systems, different level calculations, and different approaches to apply ballistics. The compressor implemented in this project is pretty much the feed-forward, peak-level, branching ballistics compressor described here: http://c4dm.eecs.qmul.ac.uk/audioengineering/compressors/documents/Reiss-Tutorialondynamicrangecompression.pdf

#### Level Detection: Why peak and not RMS?
As ballistics, that's attack and release of the gain reduction, smooth the gain reduction anyways, the implementation doesn't use RMS calculation, as it would introduce an additional time-constant. RMS calculation is not bad for compressors, they even can make it sound differently, but as this compressors should be able to be used as a limiter, we need the fast responses of a peak level detection is used here.

## The `GainReductionComputer`-class
This class is the heart of the compressor. It takes a side-chain signal and calculates the gain-reduction for each incoming sample. The gain-reduction depends on the typical parameters for compressors:
- threshold: level above which the compressor starts to reduce the gain
- knee: defines the smoothness of the linear-to-compressed-transition
- ratio: ratio between level above threshold and output-level (1 no compression, infinity limiter)
- attack: response time, how fast can the compressor keep up with transients
- release: response time, how long does it take for the compressor to release the gain reduction
- make-up: overall gain

There are a bunch of getter and setter methods which let you control the parameters. Make sure you call `prepare` with the sampleRate, so the class can convert attack and release times to samples. 

In order to compute the gain reduction, you can call `computeLinearGainFromSidechainSignal` with a pointer to the side-chain signal, a pointer where you want the gain-reduction to be written back, and the number of samples. Make sure you have enough memory allocated for the gain-reduction samples. 

Alternatively, you can call `computeGainInDecibelsFromSidechainSignal` with the same interface, and you'll get decibels levels, without the make-up gain. This is necessary if you want to alter the gain-reduction in the decibel domain, which is useful in order to implement a look-ahead. See the `LookAheadGainReduction`-class.

## The `LookAheadGainReduction` class
Use this class, if you want to add a look-ahead feature to your processor. The idea behind this class is described in the [Look-Ahead Limiter Tutorial](lookAheadLimiter.md).

It's important that you set the desired look-ahead time with `setDelayTime` and also call prepare with the sampleRate and the maximum number of samples per call. This way, the class can convert the delay time in seconds to samples, and also allocate enough memory for the delay-line.

Use the class the following way:
- call `pushSamples` to write your gain-reduction samples into the processor
- call `process` so the class will fade-in the gain-reductions
- call `readSamples` to read back the processed samples.


## The `SimpleCompressor` class
This class is a wrapper around the `GainReductionComputer`-class, so it can be easily used as a processor within the JUCE framework. Read the header-file for more information.
