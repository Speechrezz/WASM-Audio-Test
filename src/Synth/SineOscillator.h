#pragma once

#include "xylo_audio/AudioBuffer.h"
#include "xylo_audio/AudioCore.h"
#include "xylo_midi/MidiView.h"

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