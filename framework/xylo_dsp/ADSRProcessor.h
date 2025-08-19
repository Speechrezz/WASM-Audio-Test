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

        bool operator==(const Parameters& other) const;
        bool operator!=(const Parameters& other) const { return !operator==(other); }
    };

public:
    void reset() noexcept;
    void prepare(const ProcessSpec&) noexcept;
    void process(AudioView&) noexcept;
    float getNextSample() noexcept;
    
    void updateParameters(const Parameters& parametersInSeconds) noexcept;
    bool isCurrentlyPlaying() const noexcept { return state != State::off; }

    void noteOn() noexcept;
    void noteOff() noexcept;

protected:
    void updateState() noexcept;
    void setState(State state) noexcept;

    State state = State::off;
    float sampleRate = -1.0;
    Parameters parameters;
    float offset = 0.f; // Between 0 and 1
    float timeStep = 0.f;
    float attackStep = 0.f, decayStep = 0.f, releaseStep = 0.f;
    float currentValue = 0.f, startValue = 0.f, targetValue = 0.f;

};

}