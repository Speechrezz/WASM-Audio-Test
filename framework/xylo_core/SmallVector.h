/*
  ==============================================================================

    SmallVector.h
    Created: 26 Jun 2023 3:18:25pm
    Author:  Mark

  ==============================================================================
*/

#pragma once

#include "Debug.h"
#include <array>

namespace xylo
{

template<typename T, size_t maxSize>
class SmallVector
{
public:
    // Iterator class to allow range-based for loops
    class Iterator
    {
    public:
        Iterator(SmallVector* aParent, size_t newIndex)
            : parent(aParent), index(newIndex) {}

        T& operator*()
        {
            return (*parent)[index];
        }
        
        const T& operator*() const
        {
            return (*parent)[index];
        }

        T* operator->()
        {
            return &(operator*());
        }

        const T* operator->() const
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
            return index == other.index && parent == other.parent;
        }

        bool operator!=(const Iterator& other) const
        {
            return !(*this == other);
        }

        size_t getIndex() const { return index; }

    protected:
        SmallVector* parent;
        size_t index;

    };

public:
    static constexpr size_t nullIndex = static_cast<size_t>(-1);

    void clear() { currentSize = 0; }
    void pushBack(const T& newData)
    {
        XASSERT(currentSize < maxSize);
        data[currentSize++] = newData;
    }

    size_t findValue(const T& value)
    {
        for (size_t i = 0; i < size(); i++)
            if ((*this)[i] == value)
                return i;

        return nullIndex;
    }

    bool valueExists(const T& value)
    {
        return findValue(value) != nullIndex;
    }

    void remove(const size_t index)
    {
        XASSERT(index < size());

        auto* arrayStart = data.data() + index;
        const size_t copyCount = size() - index - 1;
        XASSERT(copyCount < size());

        std::memmove(arrayStart, arrayStart + 1, sizeof(T) * copyCount);
        --currentSize;
    }

    void removeValue(const T& value)
    {
        const size_t index = findValue(value);
        if (index == -1)
            return;

        remove(index);
    }

    T popBack()
    {
        XASSERT(size() > 0);

        T value = std::move((*this)[size() - 1]);
        remove(size() - 1);
        return std::move(value);
    }

    T popFront()
    {
        XASSERT(size() > 0);

        T value = std::move((*this)[0]);
        remove(0);
        return std::move(value);
    }

    void insert(const T& value, const size_t index)
    {
        XASSERT(index <= size());
        XASSERT(size() < maxSize);

        auto* copyStart = data.data() + index;
        const size_t copyCount = size() - index;
        std::memmove(copyStart + 1, copyStart, sizeof(T) * copyCount);
        data[index] = value;
        ++currentSize;
    }

    T* getRawBuffer() { return data.data(); }
    size_t size() const { return currentSize; }

    const T& operator[](const size_t index) const
    {
        XASSERT(index < size());
        return data[index];
    }

    T& operator[](const size_t index)
    {
        XASSERT(index < size());
        return data[index];
    }

    bool empty() const
    {
        return currentSize == 0;
    }

    bool full() const
    {
        return currentSize == maxSize;
    }

    // Iterator methods
    Iterator begin()
    {
        return Iterator(this, 0);
    }

    Iterator end()
    {
        return Iterator(this, currentSize);
    }

    bool operator==(const SmallVector& other)
    {
        if (currentSize != other.currentSize)
            return false;
        
        for (size_t i = 0; i < currentSize; i++)
            if ((*this)[i] != other[i])
                return false;

        return true;
    }

    bool operator!=(const SmallVector& other)
    {
        return !(*this == other);
    }

private:
    std::array<T, maxSize> data;
    size_t currentSize = 0;

};

} // namespace xynth