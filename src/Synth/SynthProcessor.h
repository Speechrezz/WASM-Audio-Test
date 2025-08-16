#pragma once

#include "Audio/AudioCore.h"
#include "Audio/AudioBuffer.h"
#include "NoteVector.h"
#include "Midi/MidiView.h"
#include "SynthVoice.h"
#include <array>

namespace xynth
{

class SynthProcessor
{
public:
    static constexpr size_t maxVoiceCount = 16;

public:
    SynthProcessor();

    void prepare(const ProcessSpec&);
    void process(AudioView& audioView, MidiView& midiView);

protected:
    void renderSynthVoices(AudioView&, int prevSamplePos, int samplesBetweenEvents);
    void handleMidiEvent(const MidiEvent&);

    void noteOn (const MidiEvent&);
    void noteOff(const MidiEvent&);

    void reclaimVoices();

protected:
    std::array<SynthVoice, maxVoiceCount> synthVoiceList;
    
    NoteVector<maxVoiceCount> activeNoteList;
    SmallVector<size_t, maxVoiceCount> inactiveVoiceList;

};

}