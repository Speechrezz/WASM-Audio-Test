#pragma once

#include <cmath>

namespace xynth
{

// Linear interpolation
template<typename T>
inline T lerp(T offset0To1, T startValue, T endValue)
{
    return offset0To1 * (endValue - startValue) + startValue;
}

// Reverse linear interpolation (maps a denormalized value to a normalized range)
template<typename T>
inline T delerp(T value, T startValue, T endValue)
{
    return (value - startValue) / (endValue - startValue);
}

// Linear mapping
template<typename T>
inline T linearMap(T valueToMap, T sourceStart, T sourceEnd, T targetStart, T targetEnd)
{
    const T normalizedValue = delerp(valueToMap, sourceStart, sourceEnd);
    return lerp(normalizedValue, targetStart, targetEnd);
}

// Maps linear value to Decibel value
template<typename T>
inline T toDecibels(T linearValue)
{
    return static_cast<T>(20.0) * std::log10(linearValue);
}

// Maps Decibel value to linear value
template<typename T>
inline T fromDecibels(T decibelValue)
{
    return std::pow(static_cast<T>(10.0), decibelValue / static_cast<T>(20.0));
}

// Maps normalized value (between 0 and 1) to target range logarithmically 
template <typename T>
T mapToLog10(T value0To1, T logRangeMin, T logRangeMax)
{
    auto logMin = std::log10(logRangeMin);
    auto logMax = std::log10(logRangeMax);

    return std::pow(static_cast<T>(10.0), value0To1 * (logMax - logMin) + logMin);
}

// Maps logarithmic range to normalized value
template <typename T>
T mapFromLog10(T valueInLogRange, T logRangeMin, T logRangeMax)
{
    auto logMin = std::log10(logRangeMin);
    auto logMax = std::log10(logRangeMax);

    return (std::log10(valueInLogRange) - logMin) / (logMax - logMin);
}

}