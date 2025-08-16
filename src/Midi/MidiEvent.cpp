#include "MidiEvent.h"
#include <cmath>

namespace xynth
{

float noteNumberToFrequency(int noteNumber, float frequencyOfA)
{
    return frequencyOfA * std::pow(2.f, static_cast<float>(noteNumber - 69) / 12.f);
}

MidiEvent::MidiEvent(uint8_t status, uint8_t d1, uint8_t d2, uint32_t samplePosition)
    : status(status), d1(d1), d2(d2), samplePosition(samplePosition)
{}

MidiEvent::MidiEvent(uint32_t packedEvent, uint32_t samplePosition) 
    : MidiEvent(
        static_cast<uint8_t>(packedEvent         & 0xff),
        static_cast<uint8_t>((packedEvent >>  8) & 0xff),
        static_cast<uint8_t>((packedEvent >> 16) & 0xff),
        samplePosition
    )
{}

float MidiEvent::getVelocityFloat() const
{
    return float(getVelocity()) * (1.f / 127.f);
}

}