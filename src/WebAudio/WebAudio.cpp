#include "WebAudio.h"

namespace xynth
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


// ---AudioBufferWASM---

void AudioBufferWASM::prepare(int numChannels)
{
    channels.resize(numChannels, nullptr);
}

AudioBufferWASM& AudioBufferWASM::operator=(const AudioSampleFrame* audioFrame)
{
    assert(audioFrame[0].numberOfChannels == channels.size());

    numSamples = audioFrame[0].samplesPerChannel;

    for (size_t i = 0; i < channels.size(); ++i)
        channels[i] = audioFrame[0].data + i * numSamples;

    return *this;
}

}