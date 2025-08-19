#include "SynthProcessor.h"

namespace xynth
{
    
SynthProcessor::SynthProcessor(xylo::AudioParameters& p) : parameters(p)
{
    for (size_t i = maxVoiceCount; i-- > 0;)
        inactiveVoiceList.pushBack(i);
}

void SynthProcessor::prepare(const xylo::ProcessSpec& spec)
{
    for (auto& synthVoice : synthVoiceList)
        synthVoice.prepare(spec);
}

void SynthProcessor::process(xylo::AudioView& audioView, xylo::MidiView& midiView)
{
    xylo::dsp::ADSRProcessor::Parameters newAdsrParameters
    {
        .attackTime  = 1e-3f * parameters.get("attack").getValue(),
        .decayTime   = 1e-3f * parameters.get("decay").getValue(),
        .sustainGain = parameters.get("sustain").getValue(),
        .releaseTime = 1e-3f * parameters.get("release").getValue()
    };

    if (adsrParameters != newAdsrParameters)
    {
        adsrParameters = newAdsrParameters;
        for (auto& voice : synthVoiceList)
            voice.updateADSR(adsrParameters);
    }

    audioView.fill(0.f);
    int prevSamplePos = 0;

    for (const auto& midiEvent : midiView)
    {
        const int samplesBetweenEvents = midiEvent.getSamplePosition() - prevSamplePos;
        renderSynthVoices(audioView, prevSamplePos, samplesBetweenEvents);

        prevSamplePos = midiEvent.getSamplePosition();
        handleMidiEvent(midiEvent);
    }

    // Finishes rendering the block of audio
    const int samplesBetweenEvents = audioView.getNumSamples() - prevSamplePos;
    renderSynthVoices(audioView, prevSamplePos, samplesBetweenEvents);
}

void SynthProcessor::renderSynthVoices(xylo::AudioView& audioView, int prevSamplePos, int samplesBetweenEvents)
{
    auto splicedView = audioView.splice(prevSamplePos, samplesBetweenEvents);
    for (auto& synthVoice : synthVoiceList)
        synthVoice.renderNextBlock(splicedView);
}

void SynthProcessor::handleMidiEvent(const xylo::MidiEvent& midiEvent)
{
    if (midiEvent.isNoteOn())
        noteOn(midiEvent);
    else if (midiEvent.isNoteOff())
        noteOff(midiEvent);
}

void SynthProcessor::noteOn(const xylo::MidiEvent& midiEvent)
{
    reclaimVoices();
    const int noteNumber = midiEvent.getNoteNumber();
    const int channel = midiEvent.getChannel();
    const float velocity = midiEvent.getVelocityFloat();

    if (activeNoteList.full())
    {
        auto oldestNote = activeNoteList.popFront();
        oldestNote.noteNumber = noteNumber;
        oldestNote.state = NoteContext::State::noteOn;
        oldestNote.channel = channel;
        activeNoteList.pushBack(oldestNote);

        synthVoiceList[oldestNote.voiceIndex].stopNote(1.f, false);
        synthVoiceList[oldestNote.voiceIndex].startNote(midiEvent.getNoteNumber(), velocity, 0);
    }
    else
    {
        const size_t voiceIndex = inactiveVoiceList.popBack();
        activeNoteList.pushBack({ noteNumber, voiceIndex, channel });
        synthVoiceList[voiceIndex].startNote(midiEvent.getNoteNumber(), velocity, 0);
    }
}

void SynthProcessor::noteOff(const xylo::MidiEvent& midiEvent)
{
    const int messageNoteNumber = midiEvent.getNoteNumber();
    const int messageChannel = midiEvent.getChannel();
    const bool isSustainPedalOn = false;

    for (auto& note : activeNoteList)
    {
        const bool isCorrectNote = messageNoteNumber == note.noteNumber;

        if (note.isOn() && isCorrectNote)
        {
            if (isSustainPedalOn)
            {
                note.state = NoteContext::State::noteSustain;
            }
            else
            {
                note.state = NoteContext::State::noteOff;
                synthVoiceList[note.voiceIndex].stopNote(1.f, true);
            }
        }
    }
}

void SynthProcessor::reclaimVoices()
{
    if (activeNoteList.empty())
        return;

    for (size_t i = activeNoteList.size(); i-- > 0;)
    {
        const auto& note = activeNoteList[i];
        if (!note.isOff() || synthVoiceList[note.voiceIndex].isCurrentPlaying())
            continue;

        inactiveVoiceList.pushBack(note.voiceIndex);
        activeNoteList.remove(i);
    }
}

}