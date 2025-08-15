#pragma once

#include "MidiEvent.h"
#include <optional>

namespace xynth
{

class MidiView
{
public:
    MidiView(const MidiEvent*, int startOffset, int numEvents);
    
    std::optional<MidiEvent> getNextEvent();

protected:
    const MidiEvent* eventBuffer;
    int numEvents, currentIndex;

};

}