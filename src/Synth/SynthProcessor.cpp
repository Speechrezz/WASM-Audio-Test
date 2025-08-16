#include "SynthProcessor.h"

namespace xynth
{
    
SynthProcessor::SynthProcessor()
{
    for (size_t i = maxVoiceCount; i-- > 0;)
        inactiveVoiceList.pushBack(i);
}

void SynthProcessor::prepare(const ProcessSpec& spec)
{
    for (auto& synthVoice : synthVoiceList)
        synthVoice.prepare(spec);
}

void SynthProcessor::process(AudioView& audioView, MidiView& midiView)
{
    audioView.fill(0.f);
    int prevSamplePos = 0;

    while (const auto midiEventOptional = midiView.getNextEvent())
    {
        const auto& midiEvent = midiEventOptional.value();
        const int samplesBetweenEvents = midiEvent.getSamplePosition() - prevSamplePos;

        renderSynthVoices(audioView, prevSamplePos, samplesBetweenEvents);

        prevSamplePos = midiEvent.getSamplePosition();
        handleMidiEvent(midiEvent);
    }

    // Finishes rendering the block of audio
    const int samplesBetweenEvents = audioView.getNumSamples() - prevSamplePos;
    renderSynthVoices(audioView, prevSamplePos, samplesBetweenEvents);
}

void SynthProcessor::renderSynthVoices(AudioView& audioView, int prevSamplePos, int samplesBetweenEvents)
{
    auto splicedView = audioView.splice(prevSamplePos, samplesBetweenEvents);
    for (auto& synthVoice : synthVoiceList)
        synthVoice.renderNextBlock(splicedView);
}

void SynthProcessor::handleMidiEvent(const MidiEvent& midiEvent)
{
    if (midiEvent.isNoteOn())
        noteOn(midiEvent);
    else if (midiEvent.isNoteOff())
        noteOff(midiEvent);
}

void SynthProcessor::noteOn(const MidiEvent& midiEvent)
{
    reclaimVoices();
    const int noteNumber = midiEvent.getNoteNumber();
    const int channel = midiEvent.getChannel();

    if (activeNoteList.full())
    {
        auto oldestNote = activeNoteList.popFront();
        oldestNote.noteNumber = noteNumber;
        oldestNote.state = NoteContext::State::noteOn;
        oldestNote.channel = channel;
        activeNoteList.pushBack(oldestNote);

        synthVoiceList[oldestNote.voiceIndex].stopNote(1.f, false);
        synthVoiceList[oldestNote.voiceIndex].startNote(midiEvent.getNoteNumber(), 1.f, 0);
    }
    else
    {
        const size_t voiceIndex = inactiveVoiceList.popBack();
        activeNoteList.pushBack({ noteNumber, voiceIndex, channel });
        synthVoiceList[voiceIndex].startNote(midiEvent.getNoteNumber(), 1.f, 0);
    }
}

void SynthProcessor::noteOff(const MidiEvent& midiEvent)
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