#include "AudioProcessor.h"
#include "Audio/AudioMath.h"

namespace xynth
{

// ---SineOscillator---

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


// ---AudioProcessor---

AudioProcessor::AudioProcessor()
{
    audioParameters.add(AudioParameter::createFrequency("frequency", "Frequency", 20.f, 2000.f, 220.f ));
    audioParameters.add(new AudioParameter{ "volume", "Volume", -60.f, 0.f, -20.f });
}

void AudioProcessor::prepare(const ProcessSpec& spec)
{
    oscillator.prepare(spec);
}

void AudioProcessor::process(AudioView& audioView)
{
    const float frequency = audioParameters.get("frequency").getValue();
    const float volume = fromDecibels(audioParameters.get("volume").getValue());
    oscillator.process(audioView, frequency, volume);
}

}