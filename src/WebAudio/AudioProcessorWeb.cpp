#include "AudioProcessorWeb.h"
#include "Midi/MidiView.h"

namespace xynth
{

// ---AudioParameterView---

AudioParameterView::AudioParameterView(const AudioParameter& p) : parameter(p)
{}

float AudioParameterView::convertToNormalizedValue(float value) const
{
	return parameter.convertToNormalizedValue(value);
}

float AudioParameterView::convertFromNormalizedValue(float normalizedValue) const
{
	return parameter.convertFromNormalizedValue(normalizedValue);
}

std::string AudioParameterView::getValueToString(float value, int maximumStringLength) const
{
	return parameter.getValueToString(value, maximumStringLength);
}

float AudioParameterView::getValueFromString(const std::string& text) const
{
	return parameter.getValueFromString(text);
}


// ---AudioProcessorWeb---

AudioProcessorWeb::AudioProcessorWeb()
{}

void AudioProcessorWeb::prepare(const ProcessSpec& spec)
{
    processor.prepare(spec);
    audioBufferWASM.prepare(spec.numChannels);
}

void AudioProcessorWeb::process(AudioSampleFrame *outputs)
{
	audioBufferWASM = outputs;
	AudioView audioView(audioBufferWASM);

	MidiView midiView(webMidi);

    processor.process(audioView, midiView);
}

void AudioProcessorWeb::updateParameters(int numParams, const AudioParamFrame *params)
{
    size_t i = 0;
	for (auto& pair : processor.audioParameters.getFullMap())
	{
		auto& parameter = *pair.second;
		parameter.setNormalizedValue(params[i++].data[0]);
	}
}

std::vector<WebAudioParamDescriptor> AudioProcessorWeb::getAudioParameterDescriptors()
{
    const auto& parameters = processor.audioParameters;
    std::vector<WebAudioParamDescriptor> descriptors;
	descriptors.reserve(parameters.getNumParameters());

	indexToParameterId.reserve(parameters.getNumParameters());

	for (const auto& pair : parameters.getFullMap())
	{
		const auto& parameter = *pair.second;

		descriptors.push_back({
			.defaultValue = parameter.convertToNormalizedValue(parameter.getDefaultValue()),
			.minValue = 0.f,
			.maxValue = 1.f,
			.automationRate = WEBAUDIO_PARAM_K_RATE
		});

		// Generate mappings
		parameterIdToIndex.insert({ parameter.getId(), (int)indexToParameterId.size() });
		indexToParameterId.push_back(parameter.getId());
	}

	return descriptors;
}

}