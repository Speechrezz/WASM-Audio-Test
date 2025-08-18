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

class AudioProcessor
{
public:
	AudioProcessor();

	void prepare(const xylo::ProcessSpec& spec);
	void process(xylo::AudioView& audioView, xylo::MidiView& midiView);

	xylo::AudioParameters audioParameters;

protected:
	xynth::SynthProcessor synth;
	xylo::dsp::GainProcessor gain;

};