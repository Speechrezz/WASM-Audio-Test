#pragma once

#include <emscripten/webaudio.h>
#include "Audio/AudioCore.h"
#include "Audio/AudioBuffer.h"
#include <vector>
#include <assert.h>

namespace xynth
{

class WebAudioContext
{
public:
    void setNodeHandle(EMSCRIPTEN_AUDIO_WORKLET_NODE_T handle);
    void setContextHandle(EMSCRIPTEN_WEBAUDIO_T handle);

    void prepare(const ProcessSpec& newSpec) { spec = newSpec; }

    double getCurrentFrame() const;
    const ProcessSpec& getProcessSpec() const { return spec; }
    
protected:
    EMSCRIPTEN_AUDIO_WORKLET_NODE_T nodeHandle;
    EMSCRIPTEN_WEBAUDIO_T contextHandle;
    ProcessSpec spec;

};

class WebAudioBuffer
{
public:
    WebAudioBuffer() = default;

    void prepare(int numChannels);
	void updateBuffer(const AudioSampleFrame* audioFrame);
    
    AudioView createView();

    int getNumChannels() const { return static_cast<int>(channels.size()); }
    int getNumSamples() const { return numSamples; }

protected:
	std::vector<float*> channels; 
	int numSamples = 0;

};

}