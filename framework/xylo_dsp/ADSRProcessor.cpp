#include "ADSRProcessor.h"

namespace xylo::dsp
{

void ADSRProcessor::reset() noexcept
{
    state = State::off;
    timePassed = 0.f;
    currentValue = 0.f;
}

void ADSRProcessor::prepare(const ProcessSpec& spec) noexcept
{
    sampleRate = float(spec.sampleRate);
    timeStep = 1.f / sampleRate;
}

void ADSRProcessor::process(AudioView& audioView) noexcept
{
    if (state == State::off)
    {
        audioView.reset();
        return;
    }

    if (state == State::sustain)
    {
        audioView.multiplyBy(parameters.sustainGain);
        return;
    }

    for (int i = 0; i < audioView.getNumSamples(); ++i)
    {
        const float envelope = getNextSample();

        for (int ch = 0; ch < audioView.getNumChannels(); ++ch)
            audioView.getChannelPointer(ch)[i] *= envelope;
    }
}

float ADSRProcessor::getNextSample() noexcept
{
    timePassed += timeStep;
    updateState();

    switch (state)
    {
    case State::off:
        return 0.f;
    case State::attack:
        return currentValue += attackStep;
    case State::decay:
        return currentValue += decayStep;
    case State::sustain:
        return currentValue = parameters.sustainGain;
    case State::release:
        return currentValue += releaseStep;
    }
}

float calculateStepSize(float difference, float duration, float sampleRate)
{
    return duration > 0.f ? (difference / (duration * sampleRate)) : 0.f;
}

void ADSRProcessor::updateParameters(Parameters parametersInSeconds) noexcept
{
    parameters = parametersInSeconds;

    attackStep  = calculateStepSize(1.f, parameters.attackTime, sampleRate);
    decayStep   = calculateStepSize(parameters.sustainGain - 1.f, parameters.decayTime, sampleRate);
    releaseStep = calculateStepSize(-parameters.sustainGain, parameters.releaseTime, sampleRate);
}

void ADSRProcessor::noteOn() noexcept
{
    if (parameters.attackTime > 0.f)
    {
        state = State::attack;
        currentValue = 0.f;
    }
    else if (parameters.decayTime > 0.f)
    {
        state = State::decay;
        currentValue = 1.f;
    }
    else
    {
        state = State::sustain;
        currentValue = parameters.sustainGain;
    }
    
    timePassed = 0.f;
}

void ADSRProcessor::noteOff() noexcept
{
    state = State::release;
    timePassed = 0.f;
    releaseStep = -currentValue / (parameters.releaseTime * sampleRate);
}

void ADSRProcessor::updateState() noexcept
{
    switch (state)
    {
    case State::attack:
        if (timePassed > parameters.attackTime)
        {
            state = State::decay;
            timePassed = 0.f;
            currentValue = 1.f;
        }
        return;
    case State::decay:
        if (timePassed > parameters.decayTime)
        {
            state = State::sustain;
            timePassed = 0.f;
            currentValue = parameters.sustainGain;
        }
        return;
    case State::release:
        if (timePassed > parameters.releaseTime)
        {
            state = State::off;
            timePassed = 0.f;
            currentValue = 0.f;
        }
        return;
    default:
        return;
    }
}
    
}