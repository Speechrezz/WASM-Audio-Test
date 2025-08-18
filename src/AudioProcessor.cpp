#include "AudioProcessor.h"
#include "xylo_audio/AudioMath.h"
#include "xylo_text/ParameterText.h"


AudioProcessor::AudioProcessor()
{
    //audioParameters.add(AudioParameter::createFrequency("frequency", "Frequency", 20.f, 2000.f, 220.f ));
    
    auto* volumeParameter = new xylo::AudioParameter{ "volume", "Volume", -60.f, 0.f, -20.f };
    volumeParameter->valueToStringMapping = [](float value, int) { return xylo::floatToString(value, 2) + " dB"; };
    audioParameters.add(volumeParameter);
}

void AudioProcessor::prepare(const xylo::ProcessSpec& spec)
{
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