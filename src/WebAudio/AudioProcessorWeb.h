#pragma once

#include "AudioProcessor.h"
#include "Audio/AudioBuffer.h"
#include "WebAudio.h"
#include <map>
#include <string>

namespace xynth
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

protected:
    const AudioParameter& parameter;

};

// AudioProcessor wrapper
class AudioProcessorWeb
{
public:
    void prepare(const ProcessSpec&);
    void process(AudioSampleFrame *outputs);

    void updateParameters(int numParams, const AudioParamFrame *params);
    std::vector<WebAudioParamDescriptor> getAudioParameterDescriptors();

    AudioProcessor& getProcessor() { return processor; }

public:
    std::map<std::string, int> parameterIdToIndex;
    std::vector<std::string> indexToParameterId;

protected:
    AudioProcessor processor;
    AudioBufferWASM audioBufferWASM;

};

}