#pragma once

#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "Audio/AudioBuffer.h"
#include "Audio/AudioParameter.h"
#include "Midi/MidiView.h"

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
	void process(AudioView& audioView, MidiView& midiView);

	AudioParameters audioParameters;

protected:
	float frequency = 220.f;
	SineOscillator oscillator;

};

}