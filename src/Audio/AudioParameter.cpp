#include "AudioParameter.h"
#include "AudioMath.h"
#include "Text/ParameterText.h"
#include <algorithm>

namespace xynth
{

// ---Parameter---

AudioParameter::AudioParameter(const std::string& parameterID,
                               const std::string& name,
                               float minValue, 
                               float maxValue, 
                               float defaultValue)
    : minValue(minValue), maxValue(maxValue), defaultValue(defaultValue), value(defaultValue),
      id(parameterID), name(name)
{
    valueToStringMapping = [](float value, int) { return floatToString(value, 2); };
    valueFromStringMapping = [](const std::string& text) { return textToValue(text); };
}

float AudioParameter::getValue() const
{
    return value;
}

float AudioParameter::getNormalizedValue() const
{
    return convertToNormalizedValue(value);
}

void AudioParameter::setValue(float newValue)
{
    value = std::clamp(newValue, minValue, maxValue);
}

void AudioParameter::setNormalizedValue(float normalizedValue)
{
    setValue(convertFromNormalizedValue(normalizedValue));
}

float AudioParameter::convertToNormalizedValue(float value) const
{
    value = std::clamp(value, minValue, maxValue);

    if (toNormalizedMapping)
        return toNormalizedMapping(minValue, maxValue, value);
    
    return delerp(value, minValue, maxValue);
}

float AudioParameter::convertFromNormalizedValue(float normalizedValue) const
{
    normalizedValue = std::clamp(normalizedValue, 0.f, 1.f);

    if (fromNormalizedMapping)
        return fromNormalizedMapping(minValue, maxValue, normalizedValue);

    return lerp(normalizedValue, minValue, maxValue);
}

std::string AudioParameter::getValueToString(float value, int maximumStringLength) const
{
    return valueToStringMapping(value, maximumStringLength);
}

float AudioParameter::getValueFromString(const std::string& text) const
{
    return valueFromStringMapping(text);
}

AudioParameter* AudioParameter::createFrequency(const std::string& parameterID,
                                               const std::string& name,
                                               float minValue,
                                               float maxValue,
                                               float defaultValue)
{
    auto* parameter = new AudioParameter(parameterID, name, minValue, maxValue, defaultValue);

    parameter->toNormalizedMapping = [](float minValue, float maxValue, float value)
    {
        return mapFromLog10(value, minValue, maxValue);
    };
    parameter->fromNormalizedMapping = [](float minValue, float maxValue, float normalizedValue)
    {
        return mapToLog10(normalizedValue, minValue, maxValue);
    };
    parameter->valueToStringMapping = [](float value, int)
    {
        return frequencyAsText(value);
    };

    return parameter;
}


// ---Parameters---

void AudioParameters::add(AudioParameter* p)
{
    parameterMap.emplace(p->getId(), p);
}

AudioParameter& AudioParameters::get(const std::string& id)
{
    return *parameterMap.at(id);
}

int AudioParameters::getNumParameters() const
{
    return (int)parameterMap.size();
}

}