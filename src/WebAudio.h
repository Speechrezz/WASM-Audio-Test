#pragma once

#include <emscripten/webaudio.h>
#include <vector>
#include <assert.h>

namespace xynth
{

class AudioBufferWASM
{
public:
    AudioBufferWASM() = default;

    void prepare(int numChannels)
    {
        channels.resize(numChannels, nullptr);
    }

	AudioBufferWASM& operator=(const AudioSampleFrame* audioFrame)
	{
        assert(audioFrame[0].numberOfChannels == channels.size());

		numSamples = audioFrame[0].samplesPerChannel;

        for (size_t i = 0; i < channels.size(); ++i)
            channels[i] = audioFrame[0].data + i * numSamples;

        return *this;
	}

    int getNumChannels() const { return static_cast<int>(channels.size()); }
    int getNumSamples() const { return numSamples; }
    float* const* getChannels() const { return channels.data(); }

protected:
	std::vector<float*> channels; 
	int numSamples = 0;

};

}