#include "WebMidi.h"
#include <iostream>

namespace xynth
{

WebMidi::WebMidi(int maxEvents) : sharedBuffer(maxEvents, 0u)
{}

void WebMidi::pushEvent(uint32_t event)
{
    const uint32_t writeIndex = getWriteIndex();
    sharedBuffer[writeIndex] = event;

    setWriteIndex((writeIndex + 1u) % getMaxEvents());
}

uint32_t WebMidi::calculateUnreadEvents(uint32_t readIndex, uint32_t writeIndex) const
{
    return (writeIndex - readIndex + getMaxEvents()) % getMaxEvents();
}

}