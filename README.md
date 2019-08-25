# SimpleCompressor
Code and theory of a look-ahead compressor/limiter.

### Purpose
The purpose of this project to demonstrate and describe, how a look-ahead limiter works and how to implement it.
About 95%<sup>1</sup> of all the sources (books, internet articles and forum threads) recommend a very easy way to implement the look-ahead feature, but unfortunately, 100%<sup>2</sup> of those applications will lose their ability to limit the signal's level.

With this, I want to shed a little light on the **mystery of look-ahead limiters** :wink:

### What's in the box?
*Clicking the following links brings you directly to the docs.*

This repository includes:
- [C++ classes](docs/classes.md)
  - computation of the gain reduction from a given side-chain signal
  - look-ahead processing of the gain reduction samples
- [JUCE class](docs/classes.md): a compressor class which wraps the gain reduction class into a JUCE audio processor
- [JUCE example projects](docs/projects.md)
  - SimpleCompressor: making use of the wrapper, implementing a compressor without look-ahead
  - LookAheadCompressor: compressor / limiter with look-ahead feature
- thirdpary classes for multi-channel samples-delays and visualization of the compressor characteristic
- a tutorial: [*How to implement look-ahead limiters*](docs/lookAheadLimiter.md)

### References
The compressor class is mainly based on this presentation:
- http://c4dm.eecs.qmul.ac.uk/audioengineering/compressors/documents/Reiss-Tutorialondynamicrangecompression.pdf

The sample delay class and visualization class is taken from the IEM Plug-in Suite
- website: https://plugins.iem.at
- source-code: https://git.iem.at/audioplugins/IEMPluginSuite


### Screenshot of the LookAheadCompressor plug-in
![alt text](docs/LookAheadCompressor.png)



###### <sup>1</sup> not based on scientific data, but estimated rather subjectively.
###### <sup>2</sup> yeah, all of them.
