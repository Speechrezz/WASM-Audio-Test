#pragma once

#include "WebAudio/WebMidi.h"
#include "MidiEvent.h"

namespace xynth
{

class MidiView
{
public:
    MidiView(WebMidi&);
    ~MidiView();
    
    bool hasNext() const { return readIndex != writeIndex; }
    MidiEvent getNextEvent();

protected:
    WebMidi& parent;
    uint32_t readIndex, writeIndex;

};

}