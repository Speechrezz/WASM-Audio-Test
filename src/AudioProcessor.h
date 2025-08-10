#pragma once

#include <emscripten/webaudio.h>
#include "AudioBuffer.h"
#include <iostream>
#include <stdlib.h>

namespace xynth
{

class AudioProcessor
{
public:
	AudioProcessor() = default;

	int getNumParams() const { return 1; }

	void prepare(const ProcessSpec& spec)
	{

	}

	void process(AudioView& audioView, float volume)
	{
		for (int channel = 0; channel < audioView.getNumChannels(); ++channel)
		{
			auto* output = audioView.getChannelPointer(channel);
			for (int i = 0; i < audioView.getNumSamples(); ++i)
			{
				output[i] = (rand() / (float)RAND_MAX * 2.0f - 1.0f) * volume; //0.1f;
			}
		}
	}

};

}