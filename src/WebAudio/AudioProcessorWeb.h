#pragma once

#include "AudioProcessor.h"
#include "AudioBuffer.h"
#include "WebAudio.h"

namespace xynth
{

// AudioProcessor wrapper
class AudioProcessorWeb
{
public:
    void prepare(const ProcessSpec&);
    void process(AudioSampleFrame *outputs);

    void updateParameters(int numParams, const AudioParamFrame *params);
    std::vector<WebAudioParamDescriptor> getAudioParameterDescriptors() const;

    AudioProcessor& getProcessor() { return processor; }

protected:
    AudioProcessor processor;
    AudioBufferWASM audioBufferWASM;

};

}