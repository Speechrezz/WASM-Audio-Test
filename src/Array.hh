#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>

template <class T>
struct Array
{
    T *array;
    size_t length, allocated;

    void Insert(T newItem, uintptr_t index)
    {
        if (length + 1 > allocated)
        {
            allocated *= 2;
            if (length + 1 > allocated)
                allocated = length + 1;
            array = (T *)realloc(array, allocated * sizeof(T));
        }

        length++;
        memmove(array + index + 1, array + index, (length - index - 1) * sizeof(T));
        array[index] = newItem;
    }

    void Delete(uintptr_t index)
    {
        memmove(array + index, array + index + 1, (length - index - 1) * sizeof(T));
        length--;
    }

    void Add(T item)
    {
        Insert(item, length);
    }
    void Free()
    {
        free(array);
        array = nullptr;
        length = allocated = 0;
    }
    int Length()
    {
        return length;
    }
    T &operator[](uintptr_t index)
    {
        assert(index < length);
        return array[index];
    }
};
