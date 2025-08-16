#include "GainProcessor.h"
#include <cmath>

namespace xynth::dsp
{

void GainProcessor::reset() noexcept
{
    value = target;
    stepCounter = 0;
}

void GainProcessor::prepare(const ProcessSpec& spec) noexcept
{
    sampleRate = spec.sampleRate;
}

void GainProcessor::process(AudioView& audioView) noexcept
{
    const float startValue = value;
    const int startCounter = stepCounter;

    for (int ch = 0; ch < audioView.getNumChannels(); ++ch)
    {
        float* channel = audioView.getChannelPointer(ch);
        value = startValue;
        stepCounter = startCounter;

        for (int i = 0; i < audioView.getNumSamples(); ++i)
            channel[i] *= getNextGainValue();
    }
}

void GainProcessor::setDurationInSeconds(double newDuration) noexcept
{
    if (newDuration == duration)
        return;

    duration = newDuration;
    numSteps = std::max(1, int(std::round(sampleRate * newDuration)));
    step = (target - value) / float(numSteps);
}

void GainProcessor::setTargetGainLinear(float gain) noexcept
{
    target = gain;
    stepCounter = numSteps;
    step = (target - value) / float(numSteps);
}

void GainProcessor::resetGainLinear(float gain) noexcept
{
    target = gain;
    value = gain;
    stepCounter = 0;
}

float GainProcessor::getNextGainValue() noexcept
{
    if (stepCounter == 0)
    {
        value = target;
    }
    else
    {
        value += step;
        --stepCounter;
    }

    return value;
}

}