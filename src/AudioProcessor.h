#include <emscripten/webaudio.h>
#include <iostream>
#include <stdlib.h>

namespace xynth
{

class AudioBufferWASM
{
public:
	AudioBufferWASM(const AudioSampleFrame* audioFrame)
	{
		numChannels = audioFrame[0].numberOfChannels;
		numSamples = audioFrame[0].samplesPerChannel;
		data = audioFrame[0].data;
	}

	int getNumChannels() const { return numChannels; }
	int getNumSamples() const { return numSamples; }

	float* getChannelPointer(int channelIndex)
	{
		return data + channelIndex * numSamples;
	}

protected:
	float* data = nullptr; 
	int numChannels = 0, numSamples = 0;

};

struct ProcessSpec
{
	int sampleRate;
	int numChannels;
	int maxBlockSize;
};

class AudioProcessor
{
public:
	AudioProcessor() = default;

	int getNumParams() const { return 1; }

	void prepare(const ProcessSpec& spec)
	{

	}

	void process(AudioBufferWASM& outputBuffer, float volume)
	{
		for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
		{
			auto* output = outputBuffer.getChannelPointer(channel);
			for (int i = 0; i < outputBuffer.getNumSamples(); ++i)
			{
				output[i] = (rand() / (float)RAND_MAX * 2.0f - 1.0f) * volume; //0.1f;
			}
		}
	}

};

}