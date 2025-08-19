#include "ADSRProcessor.h"
#include "xylo_audio/AudioMath.h"
#include "xylo_core/Debug.h"

namespace xylo::dsp
{

bool ADSRProcessor::Parameters::operator==(const Parameters& other) const
{
    return attackTime  == other.attackTime
        && decayTime   == other.decayTime
        && sustainGain == other.sustainGain
        && releaseTime == other.releaseTime;
}

void ADSRProcessor::reset() noexcept
{
    setState(State::off);
}

void ADSRProcessor::prepare(const ProcessSpec& spec) noexcept
{
    sampleRate = float(spec.sampleRate);
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
    offset += timeStep;
    updateState();

    return currentValue = lerp(offset, startValue, targetValue);
}

float calculateStepSize(float duration, float sampleRate)
{
    return duration > 0.f ? (1.f / (duration * sampleRate)) : 2.f;
}

void ADSRProcessor::updateParameters(const Parameters& parametersInSeconds) noexcept
{
    XASSERT(sampleRate > 0.0); // Call prepare()

    parameters = parametersInSeconds;

    attackStep  = calculateStepSize(parameters.attackTime,  sampleRate);
    decayStep   = calculateStepSize(parameters.decayTime,   sampleRate);
    releaseStep = calculateStepSize(parameters.releaseTime, sampleRate);
}

void ADSRProcessor::noteOn() noexcept
{
    if (parameters.attackTime > 0.f)
        setState(State::attack);
    else if (parameters.decayTime > 0.f)
        setState(State::decay);
    else
        setState(State::sustain);
}

void ADSRProcessor::noteOff() noexcept
{
    setState(State::release);
}

void ADSRProcessor::updateState() noexcept
{
    switch (state)
    {
    case State::attack:
        if (offset > 1.f)
            setState(State::decay);
        return;
    case State::decay:
        if (offset > 1.f)
            setState(State::sustain);
        return;
    case State::release:
        if (offset > 1.f)
            setState(State::off);
        return;
    default:
        return;
    }
}

void ADSRProcessor::setState(State newState) noexcept
{
    state = newState;
    offset = 0.f;

    switch (state)
    {
    case State::off:
        timeStep = 0.f;
        currentValue = 0.f;
        startValue = 0.f;
        targetValue = 0.f;
        break;
    case State::attack:
        timeStep = attackStep;
        currentValue = 0.f;
        startValue = 0.f;
        targetValue = 1.f;
        break;
    case State::decay:
        timeStep = decayStep;
        currentValue = 1.f;
        startValue = 1.f;
        targetValue = parameters.sustainGain;
        break;
    case State::sustain:
        timeStep = 0.f;
        currentValue = parameters.sustainGain;
        startValue = parameters.sustainGain;
        targetValue = parameters.sustainGain;
        break;
    case State::release:
        timeStep = releaseStep;
        startValue = currentValue;
        targetValue = 0.f;
        break;
    }
}
    
}