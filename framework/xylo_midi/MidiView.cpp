#include "MidiView.h"
#include "xylo_core/Debug.h"

namespace xylo
{

MidiView::MidiView(MidiEvent* buffer, int startOffset, int numEvents)
    : eventBuffer(buffer), startOffset(startOffset), numEvents(numEvents)
{}

MidiView::MidiView(MidiEvent* buffer, int numEvents)
    : MidiView(buffer, 0, numEvents)
{}

MidiEvent& MidiView::operator[](int index) noexcept
{
    XASSERT(index < numEvents);
    return eventBuffer[index + startOffset];
}

const MidiEvent& MidiView::operator[](int index) const noexcept
{
    XASSERT(index < numEvents);
    return eventBuffer[index + startOffset];
}

}