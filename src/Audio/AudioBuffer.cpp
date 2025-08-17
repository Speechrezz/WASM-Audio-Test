#include "AudioBuffer.h"
#include "Core/Debug.h"

namespace xynth
{


// ---AudioBuffer---

void AudioBuffer::resize(int numChannels, int newNumSamples)
{
    numSamples = newNumSamples;
    buffer.resize(numChannels * numSamples);
    
    channels.resize(numChannels);
    for (int ch = 0; ch < numChannels; ++ch)
        channels[ch] = &buffer[ch * numSamples];
}
    

// ---AudioView---

AudioView::AudioView(float* const* channels, int numChannels, int startSample, int numSamples)
    : channels(channels), numChannels(numChannels), startSample(startSample), numSamples(numSamples)
{}

AudioView::AudioView(const AudioBuffer& audioBuffer)
    : AudioView(audioBuffer.channels.data(), audioBuffer.getNumChannels(), 0, audioBuffer.getNumSamples())
{}


AudioView AudioView::splice(int newOffset, int newNumSamples) const
{
    XASSERT(startSample + newOffset + newNumSamples <= startSample + numSamples);
    return { channels, numChannels, startSample + newOffset, newNumSamples };
}

void AudioView::fill(float value)
{
    for (int ch = 0; ch < getNumChannels(); ++ch)
    {
        auto* channel = getChannelPointer(ch);
        for (int i = 0; i < getNumSamples(); ++i)
            channel[i] = value;
    }
}

void AudioView::multiplyBy(float value)
{
    for (int ch = 0; ch < getNumChannels(); ++ch)
    {
        auto* channel = getChannelPointer(ch);
        for (int i = 0; i < getNumSamples(); ++i)
            channel[i] *= value;
    }
}

void AudioView::addFrom(const AudioView& other)
{
    XASSERT(other.getNumChannels() == getNumChannels());
    XASSERT(other.getNumSamples()  == getNumSamples());
    
    for (int ch = 0; ch < getNumChannels(); ++ch)
    {
        auto* channel = getChannelPointer(ch);
        auto* channelOther = other.getChannelPointer(ch);

        for (int i = 0; i < getNumSamples(); ++i)
            channel[i] += channelOther[i];
    }
}

}