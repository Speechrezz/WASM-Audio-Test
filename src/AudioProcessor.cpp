#include "AudioProcessor.h"
#include "xylo_audio/AudioMath.h"
#include "xylo_text/ParameterText.h"


AudioProcessor::AudioProcessor() : synth(audioParameters)
{
    //audioParameters.add(AudioParameter::createFrequency("frequency", "Frequency", 20.f, 2000.f, 220.f ));
    
    auto* volumeParameter = new xylo::AudioParameter{ "volume", "Volume", -60.f, 0.f, -20.f };
    volumeParameter->valueToStringMapping = [](float value, int) { return xylo::floatToString(value, 2) + " dB"; };
    audioParameters.add(volumeParameter);

    // ---ADSR---
    auto* attackParameter = new xylo::AudioParameter{ "attack", "Attack", 0.f, 1000.f, 0.f };
    attackParameter->valueToStringMapping = xylo::msAsText;
    audioParameters.add(attackParameter);

    auto* decayParameter = new xylo::AudioParameter{ "decay", "Decay", 0.f, 1000.f, 200.f };
    decayParameter->valueToStringMapping = xylo::msAsText;
    audioParameters.add(decayParameter);

    audioParameters.add(new xylo::AudioParameter{ "sustain", "Sustain", 0.f, 1.f, 0.5f });

    auto* releaseParameter = new xylo::AudioParameter{ "release", "Release", 0.f, 1000.f, 200.f };
    releaseParameter->valueToStringMapping = xylo::msAsText;
    audioParameters.add(releaseParameter);
}

void AudioProcessor::prepare(const xylo::ProcessSpec& spec)
{
    DBG("prepare() - sampleRate: " << spec.sampleRate << ", maxBlockSize: " << spec.maxBlockSize << ", numChannels: " << spec.numChannels);
    
    synth.prepare(spec);
    gain.prepare(spec);
    gain.setDurationInSeconds(0.01);
}

void AudioProcessor::process(xylo::AudioView& audioView, xylo::MidiView& midiView)
{
    synth.process(audioView, midiView);
    
    const float volume = xylo::fromDecibels(audioParameters.get("volume").getValue());
    gain.setTargetGainLinear(volume);
    gain.process(audioView);
}