#pragma once

#include <cstdint>

namespace xynth
{

float noteNumberToFrequency(int noteNumber, float frequencyOfA = 440.f);

class MidiEvent
{
public:
    MidiEvent(uint8_t status, uint8_t d1, uint8_t d2, uint32_t samplePosition = 0);
    MidiEvent(uint32_t packedEvent, uint32_t samplePosition = 0);

    bool isNoteOn() const { return getCommand() == 0x9; }
    bool isNoteOff() const { return getCommand() == 0x8; }
    bool isNoteOnOrOff() const { return isNoteOn() || isNoteOff(); }

    int getNoteNumber() const { return static_cast<int>(d1); }
    int getVelocity() const { return static_cast<int>(d2); }
    float getVelocityFloat() const;

    uint32_t getSamplePosition() const { return samplePosition; }
    uint8_t getChannel() const { return (status & 0x0f) + 1; }

protected:
    uint8_t getCommand() const { return status >> 4; }

    uint8_t status, d1, d2;
    uint32_t samplePosition;

};

}