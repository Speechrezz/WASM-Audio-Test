#include "SynthVoice.h"

namespace xynth
{

void SynthVoice::prepare(const xylo::ProcessSpec& spec)
{
    voiceBuffer.resize(spec.numChannels, spec.maxBlockSize);
    osc.prepare(spec);

    adsr.prepare(spec);
    adsr.updateParameters({ 0.f, 0.5f, 0.5f, 0.2f });
}

void SynthVoice::renderNextBlock(xylo::AudioView& outputView)
{
    if (!isCurrentPlaying() || outputView.getNumSamples() == 0)
        return;

    auto voiceView = xylo::AudioView(voiceBuffer).splice(0, outputView.getNumSamples());
    osc.process(voiceView, frequency, velocity);
    adsr.process(voiceView);

    outputView.addFrom(voiceView);
}

void SynthVoice::startNote(int midiNoteNumber, float newVelocity, int)
{
    frequency = xylo::noteNumberToFrequency(midiNoteNumber);
    velocity = newVelocity;

    adsr.noteOn();
    osc.reset();

    noteOnFlag = true;
}

void SynthVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
        adsr.reset();

    noteOnFlag = false;
}

}