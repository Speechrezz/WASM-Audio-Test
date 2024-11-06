#pragma once
#include <cassert>
#ifdef __EMSCRIPTEN__
#include <emscripten/webaudio.h>
#else
#include "../external/clap/include/clap/clap.h"
#endif

#include <iostream>
#include <stdlib.h>

namespace xynth
{
#ifdef __EMSCRIPTEN__
class AudioBuffer
{
  public:
    AudioBuffer(const AudioSampleFrame *audioFrame)
    {
        numChannels = audioFrame[0].numberOfChannels;
        numSamples = audioFrame[0].samplesPerChannel;
        data = audioFrame[0].data;
    }

    int getNumChannels() const
    {
        return numChannels;
    }
    int getNumSamples() const
    {
        return numSamples;
    }

    float *getChannelPointer(int channelIndex)
    {
        return data + channelIndex * numSamples;
    }

  protected:
    float *data = nullptr;
    int numChannels = 0, numSamples = 0;
};
#else
// Stolen from https://github.com/emscripten-core/emscripten/blob/main/system/include/emscripten/webaudio.h
// typedef struct AudioSampleFrame
// {
//     const int numberOfChannels;
//     const int samplesPerChannel;
//     float *data;
// } AudioSampleFrame;
//
class AudioBuffer
{
  public:
    AudioBuffer() = default;

    void resize(const clap_process_t *audioFrame)
    {
        numChannels = audioFrame[0].audio_outputs[0].channel_count;
        data.resize(numChannels);
        numSamples = audioFrame[0].frames_count;
        for (int i = 0; i < numChannels; i++)
        {
            data[i] = audioFrame[0].audio_outputs[0].data32[i];
        }
    }

    int getNumChannels() const
    {
        return numChannels;
    }
    int getNumSamples() const
    {
        return numSamples;
    }

    float *getChannelPointer(int channelIndex)
    {
        return data[channelIndex];
    }

  protected:
    std::vector<float *> data;
    int numChannels = 0, numSamples = 0;
};
#endif

struct ProcessSpec
{
    int sampleRate;
    int numChannels;
    int maxBlockSize;
};

class AudioProcessor
{
  public:
    AudioProcessor() = default;

    int getNumParams() const
    {
        return 1;
    }

    void prepare(const ProcessSpec &spec)
    {
    }

    void process(AudioBuffer &outputBuffer, float volume)
    {
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            auto *output = outputBuffer.getChannelPointer(channel);
            for (int i = 0; i < outputBuffer.getNumSamples(); ++i)
            {
                output[i] = (rand() / (float)RAND_MAX * 2.0f - 1.0f) * volume; // 0.1f;
            }
        }
    }
};

} // namespace xynth
