#pragma once

#include "MidiEvent.h"
#include <cstddef>

namespace xynth
{

class MidiView
{
public:
    // Iterator class to allow range-based for loops
    class Iterator
    {
    public:
        Iterator(MidiView& parent, size_t index)
            : parent(parent), index(index) {}

        MidiEvent& operator*()
        {
            return parent[index];
        }
        
        const MidiEvent& operator*() const
        {
            return parent[index];
        }

        MidiEvent* operator->()
        {
            return &(operator*());
        }

        const MidiEvent* operator->() const
        {
            return &(operator*());
        }

        Iterator& operator++()
        {
            ++index;
            return *this;
        }

        bool operator==(const Iterator& other) const
        {
            return index == other.index && &parent == &(other.parent);
        }

        bool operator!=(const Iterator& other) const
        {
            return !(*this == other);
        }

        size_t getIndex() const { return index; }

    protected:
        MidiView& parent;
        size_t index;

    };

public:
    MidiView(MidiEvent*, int startOffset, int numEvents);
    MidiView(MidiEvent*, int numEvents);
    
    MidiEvent& operator[](int index) noexcept;
    const MidiEvent& operator[](int index) const noexcept;

    int getNumEvents() const { return numEvents; }

    Iterator begin() { return Iterator(*this, 0); }
    Iterator end()   { return Iterator(*this, numEvents); }

protected:
    MidiEvent* eventBuffer;
    int startOffset, numEvents;

};

}