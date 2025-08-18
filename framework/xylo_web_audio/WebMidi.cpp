#include "WebMidi.h"
#include <iostream>
#include <cmath>

namespace xylo
{

WebMidi::WebMidi(const WebAudioContext& c, int maxEvents) 
    : audioContext(c), sharedBuffer(maxEvents, { 0u, 0u })
{
    localBuffer.reserve(maxEvents);
}

MidiView WebMidi::createView()
{
    auto& eventBuffer = readEvents();
    return { eventBuffer.data(), 0, static_cast<int>(eventBuffer.size()) };
}

void WebMidi::pushEvent(uint32_t event, uint32_t timeStamp)
{
    const uint32_t writeIndex = getWriteIndex();
    sharedBuffer[writeIndex] = { event, timeStamp };

    setWriteIndex((writeIndex + 1u) % getMaxEvents());
}

std::vector<MidiEvent>& WebMidi::readEvents()
{
    uint32_t readIndex = getReadIndex();
    const uint32_t writeIndex = getWriteIndex();

    localBuffer.clear();
    while (readIndex != writeIndex)
    {
        const auto packedEvent = getEvent(readIndex);
        if (packedEvent.timeStamp >= audioContext.getProcessSpec().maxBlockSize)
            break;

        localBuffer.emplace_back(packedEvent.event, packedEvent.timeStamp);
        readIndex = (readIndex + 1) % getMaxEvents();
    }

    setReadIndex(readIndex);
    return localBuffer;
}

uint32_t WebMidi::calculateUnreadEvents(uint32_t readIndex, uint32_t writeIndex) const
{
    return (writeIndex - readIndex + getMaxEvents()) % getMaxEvents();
}

WebMidi::PackedEvent WebMidi::getEvent(uint32_t index) const
{
    auto adjustedEvent = sharedBuffer[index];
    const auto currentFrame = static_cast<int64_t>(audioContext.getCurrentFrame());

    const int64_t delayedTimeStamp = adjustedEvent.timeStamp + audioContext.getProcessSpec().maxBlockSize;
    const int64_t sampleOffset = std::max(static_cast<int64_t>(0), delayedTimeStamp - currentFrame);
    adjustedEvent.timeStamp = static_cast<uint32_t>(sampleOffset);

    return adjustedEvent;
}

}