#pragma once

#include "xylo_audio/AudioCore.h"
#include "xylo_audio/AudioBuffer.h"

namespace xylo::dsp
{

class ADSRProcessor
{
public:
    enum class State
    {
        off, attack, decay, sustain, release
    };

    struct Parameters
    {
        float attackTime, decayTime, sustainGain, releaseTime;
    };

public:
    void reset() noexcept;
    void prepare(const ProcessSpec&) noexcept;
    void process(AudioView&) noexcept;
    float getNextSample() noexcept;
    
    void updateParameters(Parameters parametersInSeconds) noexcept;
    bool isCurrentlyPlaying() const noexcept { return state != State::off; }

    void noteOn() noexcept;
    void noteOff() noexcept;

protected:
    void updateState() noexcept;

    State state = State::off;
    float sampleRate = -1.0;
    Parameters parameters;
    float attackStep = 0.f, decayStep = 0.f, releaseStep = 0.f;
    float timePassed = 0.f, timeStep = 0.f;
    float currentValue = 0.f;

};

}