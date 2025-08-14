#pragma once

#include <emscripten/webaudio.h>
#include <vector>
#include <atomic>

namespace xynth
{

class WebMidi
{
public:
    WebMidi(int maxEvents = 512);

    void pushEvent(uint32_t event);
    uint32_t operator[](uint32_t index) const { return sharedBuffer[index]; }

    uint32_t getMaxEvents() const { return static_cast<uint32_t>(sharedBuffer.size()); }

    uint32_t getReadIndex()  const { return readIndexAtomic .load(std::memory_order_relaxed); }
    uint32_t getWriteIndex() const { return writeIndexAtomic.load(std::memory_order_relaxed); }

    void setReadIndex (uint32_t newIndex) { readIndexAtomic .store(newIndex, std::memory_order_relaxed); }
    void setWriteIndex(uint32_t newIndex) { writeIndexAtomic.store(newIndex, std::memory_order_relaxed); }
    
    uint32_t calculateUnreadEvents(uint32_t readIndex, uint32_t writeIndex) const;

protected:
    std::vector<uint32_t> sharedBuffer;
    std::atomic<uint32_t> readIndexAtomic{ 0 }, writeIndexAtomic{ 0 };

};

}