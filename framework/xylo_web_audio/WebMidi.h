#pragma once

#include <emscripten/webaudio.h>
#include "WebAudio.h"
#include "xylo_midi/MidiView.h"
#include <vector>
#include <atomic>

namespace xynth
{

class WebMidi
{
public:
    struct PackedEvent
    {
        uint32_t event, timeStamp;
    };

public:
    WebMidi(const WebAudioContext&, int maxEvents = 512);

    MidiView createView();

    void pushEvent(uint32_t event, uint32_t timeStamp);
    std::vector<MidiEvent>& readEvents();

    uint32_t getMaxEvents() const { return static_cast<uint32_t>(sharedBuffer.size()); }

    uint32_t getReadIndex()  const { return readIndexAtomic .load(std::memory_order_acquire); }
    uint32_t getWriteIndex() const { return writeIndexAtomic.load(std::memory_order_acquire); }

    void setReadIndex (uint32_t newIndex) { readIndexAtomic .store(newIndex, std::memory_order_release); }
    void setWriteIndex(uint32_t newIndex) { writeIndexAtomic.store(newIndex, std::memory_order_release); }
    
    uint32_t calculateUnreadEvents(uint32_t readIndex, uint32_t writeIndex) const;
    uint32_t getMaxBlockSize() const { return audioContext.getProcessSpec().maxBlockSize; }

protected:
    PackedEvent getEvent(uint32_t index) const;

    const WebAudioContext& audioContext;
    std::vector<PackedEvent> sharedBuffer;
    std::vector<MidiEvent> localBuffer;
    std::atomic<uint32_t> readIndexAtomic{ 0 }, writeIndexAtomic{ 0 };

};

}