#include "WebAudio.h"
#include "xylo_core/Debug.h"

namespace xylo
{


// ---WebAudioContext---

void WebAudioContext::setNodeHandle(EMSCRIPTEN_AUDIO_WORKLET_NODE_T handle)
{
    nodeHandle = handle;
}

void WebAudioContext::setContextHandle(EMSCRIPTEN_WEBAUDIO_T handle)
{
    contextHandle = handle;
}

double WebAudioContext::getCurrentFrame() const
{
    return EM_ASM_DOUBLE({
        return currentFrame;
    });
}


// ---WebAudioBuffer---

void WebAudioBuffer::prepare(int numChannels)
{
    channels.resize(numChannels, nullptr);
}

void WebAudioBuffer::updateBuffer(const AudioSampleFrame* audioFrame)
{
    XASSERT(audioFrame[0].numberOfChannels == channels.size());

    numSamples = audioFrame[0].samplesPerChannel;

    for (size_t i = 0; i < channels.size(); ++i)
        channels[i] = audioFrame[0].data + i * numSamples;
}

AudioView WebAudioBuffer::createView()
{
    return AudioView(channels.data(), getNumChannels(), 0, getNumSamples());
}

}