#pragma once

#include "WebAudio.h"

namespace xynth
{

struct ProcessSpec
{
	int sampleRate;
	int numChannels;
	int maxBlockSize;
};

class AudioView
{
public:
	AudioView(const AudioBufferWASM& audioBufferWASM)
	{
		numChannels = audioBufferWASM.getNumChannels();
		numSamples = audioBufferWASM.getNumSamples();
		channels = audioBufferWASM.getChannels();
	}

	int getNumChannels() const { return numChannels; }
	int getNumSamples() const { return numSamples; }

	float* getChannelPointer(int channelIndex)
	{
		return channels[channelIndex];
	}

protected:
	float* const* channels = nullptr; 
	int numChannels = 0, startSample = 0, numSamples = 0;

};

}