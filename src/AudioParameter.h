#pragma once

#include <map>
#include <string>

namespace xynth
{

class AudioParameter
{
public:
    AudioParameter(float minValue, float maxValue, float defaultValue);

    float getValue() const;
    float getNormalizedValue() const;
    float getMinValue() const { return minValue; }
    float getMaxValue() const { return maxValue; }
    float getDefaultValue() const { return defaultValue; }

    void setValue(float);
    void setNormalizedValue(float);

protected:
    float minValue, maxValue, defaultValue;
    float value;

};

class AudioParameters
{
public:
    void add(const std::string& id, const AudioParameter&);
    AudioParameter& get(const std::string& id);

    int getNumParameters() const;

    std::map<std::string, AudioParameter>& getFullMap() { return parameterMap; }

private:
    std::map<std::string, AudioParameter> parameterMap;

};

}