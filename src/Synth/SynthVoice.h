#pragma once

#include "SineOscillator.h"
#include "xylo_audio/AudioBuffer.h"
#include "xylo_dsp/GainProcessor.h"

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
        return !gain.isSilent(); 
    }

protected:
    xylo::AudioBuffer voiceBuffer;
    SineOscillator osc;
    xylo::dsp::GainProcessor gain;
    float frequency = 0.f, velocity = 1.f;
    bool noteOnFlag = false;

};

}