#pragma once

#include <emscripten/webaudio.h>
#include "AudioBuffer.h"
#include "AudioParameter.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>

namespace xynth
{

class SineOscillator
{
public:
	SineOscillator() = default;

	void prepare(const ProcessSpec& spec);
	void process(AudioView& audioView, float frequency, float volume);

protected:
	float radiansCoefficient = 0.f;
	float phase = 0.f;

};

class AudioProcessor
{
public:
	AudioProcessor();

	void prepare(const ProcessSpec& spec);
	void process(AudioView& audioView);

	AudioParameters audioParameters;

protected:
	SineOscillator oscillator;

};

}