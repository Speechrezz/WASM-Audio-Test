#pragma once

#include "xylo_audio/AudioBuffer.h"
#include "xylo_audio/AudioCore.h"

namespace xylo::dsp
{

class GainProcessor
{
public:
    void reset() noexcept;
    void prepare(const ProcessSpec&) noexcept;
    void process(AudioView&) noexcept;

    void setDurationInSeconds(double duration) noexcept;
    void setTargetGainLinear(float gain) noexcept;
    void resetGainLinear(float gain) noexcept;

    bool isSmoothing() const noexcept { return stepCounter > 0; }
    bool isSilent() const noexcept { return !isSmoothing() && value == 0.f; }

protected:
    float getNextGainValue() noexcept;

    float value = 0.f, target = 0.f, step = 1.f;
    double sampleRate = -1.0, duration = 0.0;
    int numSteps = 1, stepCounter = 0;

};

}