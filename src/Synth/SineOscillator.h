#pragma once

#include "Audio/AudioBuffer.h"
#include "Audio/AudioCore.h"
#include "Midi/MidiView.h"

namespace xynth
{

class SineOscillator
{
public:
	SineOscillator() = default;

	void reset();
	void prepare(const ProcessSpec& spec);
	void process(AudioView& audioView, float frequency, float volume);

protected:
	float radiansCoefficient = 0.f;
	float phase = 0.f;

};

}