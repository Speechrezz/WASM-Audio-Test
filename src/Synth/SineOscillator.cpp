#include "SineOscillator.h"

namespace xynth
{

void SineOscillator::reset()
{
    phase = 0.f;
}

void SineOscillator::prepare(const ProcessSpec& spec)
{
    phase = 0.f;
    radiansCoefficient = 2.f * M_PI / float(spec.sampleRate);
}

void SineOscillator::process(AudioView& audioView, float frequency, float volume)
{
    const float phaseOffset = frequency * radiansCoefficient;
    float currentPhase = phase;

    for (int ch = 0; ch < audioView.getNumChannels(); ++ch)
    {
        currentPhase = phase;
        auto* channel = audioView.getChannelPointer(ch);

        for (int i = 0; i < audioView.getNumSamples(); ++i)
        {
            channel[i] = std::sin(currentPhase) * volume;
            currentPhase += phaseOffset;
        }
    }

    phase = std::fmod(currentPhase, 2.f * M_PI);
}

}