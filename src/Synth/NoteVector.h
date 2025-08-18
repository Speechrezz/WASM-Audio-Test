/*
  ==============================================================================

    NoteVector.h
    Created: 24 Aug 2024 11:03:06am
    Author:  Mark

  ==============================================================================
*/

#pragma once

#include "xylo_core/SmallVector.h"
#include <cstdint>
#include <string>

namespace xynth
{

struct NoteContext
{
    int noteNumber;
    size_t voiceIndex;
    int channel;

    enum class State : char { noteOff, noteOn, noteSustain };
    State state = State::noteOn;

    bool isOff() const { return state == State::noteOff; }
    bool isOn()  const { return state == State::noteOn; }
    bool isSustain() const { return state == State::noteSustain; }
    bool isOnOrSustain() const { return isOn() || isSustain(); }

    std::string stateToString() const
    {
        switch (state)
        {
        default:
        case State::noteOff:
            return "off";
        case State::noteOn:
            return "on ";
        case State::noteSustain:
            return "sus";
        }
    }
};

template<size_t maxSize>
class NoteVector : public xylo::SmallVector<NoteContext, maxSize>
{
public:
    size_t findNote(int noteNumber)
    {
        for (size_t i = 0; i < this->size(); i++)
            if ((*this)[i].noteNumber == noteNumber)
                return i;

        return this->nullIndex;
    }

    size_t findVoiceIndex(size_t voiceIndex)
    {
        for (size_t i = 0; i < this->size(); i++)
            if ((*this)[i].voiceIndex == voiceIndex)
                return i;

        return this->nullIndex;
    }

    bool noteExists(int noteNumber)
    {
        return findNote(noteNumber) != this->nullIndex;
    }

    void removeNote(int noteNumber)
    {
        const size_t index = findNote(noteNumber);
        if (index != this->nullIndex)
            this->remove(index);
    }

    void removeVoiceIndex(size_t voiceIndex)
    {
        const size_t index = findVoiceIndex(voiceIndex);
        if (index != this->nullIndex)
            this->remove(index);
    }

};

}