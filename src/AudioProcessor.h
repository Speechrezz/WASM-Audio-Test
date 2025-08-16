#pragma once

#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "Audio/AudioBuffer.h"
#include "Audio/AudioCore.h"
#include "Audio/AudioParameter.h"
#include "Dsp/GainProcessor.h"
#include "Midi/MidiView.h"
#include "Synth/SynthProcessor.h"

namespace xynth
{

class AudioProcessor
{
public:
	AudioProcessor();

	void prepare(const ProcessSpec& spec);
	void process(AudioView& audioView, MidiView& midiView);

	AudioParameters audioParameters;

protected:
	SynthProcessor synth;
	dsp::GainProcessor gain;

};

}