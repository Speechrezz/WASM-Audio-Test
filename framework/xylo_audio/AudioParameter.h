#pragma once

#include <map>
#include <string>
#include <functional>
#include <memory>

namespace xynth
{

class AudioParameter
{
public:
    AudioParameter(const std::string& parameterID,
                   const std::string& name,
                   float minValue, 
                   float maxValue, 
                   float defaultValue);

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }

    float getValue() const;
    float getNormalizedValue() const;
    float getMinValue() const { return minValue; }
    float getMaxValue() const { return maxValue; }
    float getDefaultValue() const { return defaultValue; }

    void setValue(float);
    void setNormalizedValue(float);

    float convertToNormalizedValue(float value) const;
    float convertFromNormalizedValue(float normalizedValue) const;

    std::string getValueToString(float value, int maximumStringLength = 2) const;
    float getValueFromString(const std::string& text) const;

    std::function<std::string(float, int)> valueToStringMapping;
    std::function<float(const std::string&)> valueFromStringMapping;

    std::function<float(float minValue, float maxValue, float value)> toNormalizedMapping;
    std::function<float(float minValue, float maxValue, float normalizedValue)> fromNormalizedMapping;

protected:
    std::string id, name;
    float minValue, maxValue, defaultValue;
    float value;

public: // Special Parameter types
    static AudioParameter* createFrequency(const std::string& parameterID,
                                          const std::string& name,
                                          float minValue,
                                          float maxValue,
                                          float defaultValue);

};

class AudioParameters
{
public:
    using ParameterMap = std::map<std::string, std::unique_ptr<AudioParameter>>;

    void add(AudioParameter*);
    AudioParameter& get(const std::string& id);

    int getNumParameters() const;

    ParameterMap& getFullMap() { return parameterMap; }
    const ParameterMap& getFullMap() const { return parameterMap; }

private:
    ParameterMap parameterMap;

};

}