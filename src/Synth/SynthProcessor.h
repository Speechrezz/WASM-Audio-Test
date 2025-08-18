#pragma once

#include "xylo_audio/AudioCore.h"
#include "xylo_audio/AudioBuffer.h"
#include "NoteVector.h"
#include "xylo_midi/MidiView.h"
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

    void prepare(const xylo::ProcessSpec&);
    void process(xylo::AudioView& audioView, xylo::MidiView& midiView);

protected:
    void renderSynthVoices(xylo::AudioView&, int prevSamplePos, int samplesBetweenEvents);
    void handleMidiEvent(const xylo::MidiEvent&);

    void noteOn (const xylo::MidiEvent&);
    void noteOff(const xylo::MidiEvent&);

    void reclaimVoices();

protected:
    std::array<SynthVoice, maxVoiceCount> synthVoiceList;
    
    NoteVector<maxVoiceCount> activeNoteList;
    xylo::SmallVector<size_t, maxVoiceCount> inactiveVoiceList;

};

}