#include "SynthVoice.h"

namespace xynth
{

void SynthVoice::prepare(const ProcessSpec& spec)
{
    voiceBuffer.resize(spec.numChannels, spec.maxBlockSize);
    osc.prepare(spec);
}

void SynthVoice::renderNextBlock(AudioView& outputView)
{
    if (!isCurrentPlaying() || outputView.getNumSamples() == 0)
        return;

    auto voiceView = AudioView(voiceBuffer).splice(0, outputView.getNumSamples());
    osc.process(voiceView, frequency, velocity);

    outputView.addFrom(voiceView);
}

void SynthVoice::startNote(int midiNoteNumber, float newVelocity, int)
{
    frequency = noteNumberToFrequency(midiNoteNumber);
    velocity = newVelocity;
    osc.reset();
    noteOnFlag = true;
}

void SynthVoice::stopNote(float, bool allowTailOff)
{
    noteOnFlag = false;
}

}