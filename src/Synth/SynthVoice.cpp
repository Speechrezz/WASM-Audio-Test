#include "SynthVoice.h"

namespace xynth
{

void SynthVoice::prepare(const ProcessSpec& spec)
{
    voiceBuffer.resize(spec.numChannels, spec.maxBlockSize);
    osc.prepare(spec);

    gain.prepare(spec);
    gain.setDurationInSeconds(0.002);
}

void SynthVoice::renderNextBlock(AudioView& outputView)
{
    if (!isCurrentPlaying() || outputView.getNumSamples() == 0)
        return;

    auto voiceView = AudioView(voiceBuffer).splice(0, outputView.getNumSamples());
    osc.process(voiceView, frequency, velocity);
    gain.process(voiceView);

    outputView.addFrom(voiceView);
}

void SynthVoice::startNote(int midiNoteNumber, float newVelocity, int)
{
    frequency = noteNumberToFrequency(midiNoteNumber);
    velocity = newVelocity;
    osc.reset();

    gain.resetGainLinear(1.f);

    noteOnFlag = true;
}

void SynthVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        gain.setTargetGainLinear(0.f);
    else
        gain.resetGainLinear(0.f);

    noteOnFlag = false;
}

}