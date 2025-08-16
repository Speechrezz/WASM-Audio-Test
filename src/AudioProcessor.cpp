#include "AudioProcessor.h"
#include "Audio/AudioMath.h"
#include "Text/ParameterText.h"

namespace xynth
{


// ---AudioProcessor---

AudioProcessor::AudioProcessor()
{
    //audioParameters.add(AudioParameter::createFrequency("frequency", "Frequency", 20.f, 2000.f, 220.f ));
    
    auto* volumeParameter = new AudioParameter{ "volume", "Volume", -60.f, 0.f, -20.f };
    volumeParameter->valueToStringMapping = [](float value, int) { return floatToString(value, 2) + " dB"; };
    audioParameters.add(volumeParameter);
}

void AudioProcessor::prepare(const ProcessSpec& spec)
{
    synth.prepare(spec);
    gain.prepare(spec);
    gain.setDurationInSeconds(0.01);
}

void AudioProcessor::process(AudioView& audioView, MidiView& midiView)
{
    const float volume = fromDecibels(audioParameters.get("volume").getValue());
    gain.setTargetGainLinear(volume);

    synth.process(audioView, midiView);
    gain.process(audioView);
}

}