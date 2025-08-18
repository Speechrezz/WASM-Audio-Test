#pragma once

#include "SineOscillator.h"
#include "xylo_audio/AudioBuffer.h"
#include "xylo_dsp/ADSRProcessor.h"

namespace xynth
{

class SynthVoice
{
public:
    void prepare(const xylo::ProcessSpec&);
    void renderNextBlock(xylo::AudioView& audioView);

    void startNote(int midiNoteNumber, float velocity, int currentPitchWheelPosition);
    void stopNote(float velocity, bool allowTailOff = true);

    bool isCurrentPlaying() const 
    { 
        return adsr.isCurrentlyPlaying(); 
    }

protected:
    xylo::AudioBuffer voiceBuffer;
    SineOscillator osc;
    xylo::dsp::ADSRProcessor adsr;
    float frequency = 0.f, velocity = 1.f;
    bool noteOnFlag = false;

};

}