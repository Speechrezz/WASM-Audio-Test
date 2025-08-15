#include "MidiView.h"
#include <emscripten.h>

namespace xynth
{

MidiView::MidiView(const MidiEvent* buffer, int startOffset, int numEvents)
    : eventBuffer(buffer), currentIndex(startOffset), numEvents(numEvents)
{}

std::optional<MidiEvent> MidiView::getNextEvent()
{
    if (currentIndex == numEvents)
        return std::nullopt;

    return eventBuffer[currentIndex++];
}

}