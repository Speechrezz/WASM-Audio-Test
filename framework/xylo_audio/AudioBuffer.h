#pragma once

#include <vector>

namespace xylo
{

class AudioView;

class AudioBuffer
{
public:
	void resize(int numChannels, int numSamples);

	int getNumChannels() const { return int(channels.size()); }
	int getNumSamples()  const { return numSamples; }

	float* const getChannelPointer(int channelIndex) const
	{
		return channels[channelIndex];
	}

protected:
	friend AudioView;
	std::vector<float> buffer;
	std::vector<float*> channels;
	int numSamples = 0;

};

class AudioView
{
public:
	AudioView(float* const* channels, int numChannels, int startSample, int numSamples);
	AudioView(const AudioBuffer& audioBuffer);

	AudioView splice(int startOffset, int numSamples) const;

	void reset();
	void fill(float value);
	void multiplyBy(float value);

	void addFrom(const AudioView& other);

	int getNumChannels() const { return numChannels; }
	int getNumSamples() const { return numSamples; }

	float* const getChannelPointer(int channelIndex) const
	{
		return channels[channelIndex] + startSample;
	}

protected:
	float* const* channels = nullptr; 
	int numChannels = 0, startSample = 0, numSamples = 0;

};

}