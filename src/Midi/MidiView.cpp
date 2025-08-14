#include "MidiView.h"

namespace xynth
{

MidiView::MidiView(WebMidi& p) : parent(p)
{
    readIndex = parent.getReadIndex();
    writeIndex = parent.getWriteIndex();
}

MidiView::~MidiView()
{
    parent.setReadIndex(readIndex);
}

MidiEvent MidiView::getNextEvent()
{
    MidiEvent midiEvent(parent[readIndex]);
    readIndex = (readIndex + 1) % parent.getMaxEvents();
    return midiEvent;
}

}