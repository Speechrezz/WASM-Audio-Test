#pragma once

#include <map>
#include <string>
#include "WebMidi.h"
#include "WebAudio.h"
#include "AudioProcessor.h"
#include "xylo_audio/AudioBuffer.h"

namespace xylo
{

// AudioParameter wrapper
class AudioParameterView
{
public:
    AudioParameterView(const AudioParameter&);

    const std::string& getName() const { return parameter.getName(); }
    const std::string& getId() const { return parameter.getId(); }

    float convertToNormalizedValue(float value) const;
    float convertFromNormalizedValue(float normalizedValue) const;

    std::string getValueToString(float value, int maximumStringLength = 2) const;
    float getValueFromString(const std::string& text) const;

protected:
    const AudioParameter& parameter;

};

// AudioProcessor wrapper
class AudioProcessorWeb
{
public:
    AudioProcessorWeb();

    void prepare(const ProcessSpec&);
    void process(AudioSampleFrame *outputs);

    void updateParameters(int numParams, const AudioParamFrame *params);
    std::vector<WebAudioParamDescriptor> getAudioParameterDescriptors();

    AudioProcessor& getProcessor() { return processor; }

public:
    std::map<std::string, int> parameterIdToIndex;
    std::vector<std::string> indexToParameterId;
    WebMidi webMidi;
    WebAudioContext audioContext;

protected:
    AudioProcessor processor;
    WebAudioBuffer webAudioBuffer;

};

}