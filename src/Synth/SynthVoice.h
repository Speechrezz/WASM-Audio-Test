#pragma once

#include "SineOscillator.h"
#include "Audio/AudioBuffer.h"

namespace xynth
{

class SynthVoice
{
public:
    void prepare(const ProcessSpec&);
    void renderNextBlock(AudioView& audioView);

    void startNote(int midiNoteNumber, float velocity, int currentPitchWheelPosition);
    void stopNote(float velocity, bool allowTailOff = true);

    bool isCurrentPlaying() const { return noteOnFlag; }

protected:
    AudioBuffer voiceBuffer;
    SineOscillator osc;
    float frequency = 0.f, velocity = 1.f;
    bool noteOnFlag = false;

};

}