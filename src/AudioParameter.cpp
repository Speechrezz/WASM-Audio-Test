#include "AudioParameter.h"

namespace xynth
{

// ---Parameter---

AudioParameter::AudioParameter(float minValue, float maxValue, float defaultValue)
    : minValue(minValue), maxValue(maxValue), defaultValue(defaultValue), value(defaultValue)
{}

float AudioParameter::getValue() const
{
    return value;
}

float AudioParameter::getNormalizedValue() const
{
    float normalizedValue = value - minValue;
    return normalizedValue / (maxValue - minValue);
}

void AudioParameter::setValue(float newValue)
{
    value = newValue;
}

void AudioParameter::setNormalizedValue(float normalizedValue)
{
    float newValue = normalizedValue * (maxValue - minValue);
    value = newValue + minValue;
}


// ---Parameters---

void AudioParameters::add(const std::string& id, const AudioParameter& p)
{
    parameterMap.insert({id, p});
}

AudioParameter& AudioParameters::get(const std::string& id)
{
    return parameterMap.at(id);
}

int AudioParameters::getNumParameters() const
{
    return (int)parameterMap.size();
}

}