#pragma once

#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "xylo_audio/AudioBuffer.h"
#include "xylo_audio/AudioCore.h"
#include "xylo_audio/AudioParameter.h"
#include "xylo_dsp/GainProcessor.h"
#include "xylo_midi/MidiView.h"
#include "synth/SynthProcessor.h"

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