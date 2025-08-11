#include "AudioProcessorWeb.h"

namespace xynth
{

void AudioProcessorWeb::prepare(const ProcessSpec& spec)
{
    processor.prepare(spec);
    audioBufferWASM.prepare(spec.numChannels);
}

void AudioProcessorWeb::process(AudioSampleFrame *outputs)
{
	audioBufferWASM = outputs;
	AudioView audioView(audioBufferWASM);
    processor.process(audioView);
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

std::vector<WebAudioParamDescriptor> AudioProcessorWeb::getAudioParameterDescriptors() const
{
    const auto& parameters = processor.audioParameters;
    std::vector<WebAudioParamDescriptor> descriptors;
	descriptors.reserve(parameters.getNumParameters());

	for (const auto& pair : parameters.getFullMap())
	{
		const auto& parameter = *pair.second;

		descriptors.push_back({
			.defaultValue = parameter.convertToNormalizedValue(parameter.getDefaultValue()),
			.minValue = 0.f,
			.maxValue = 1.f,
			.automationRate = WEBAUDIO_PARAM_K_RATE
		});
	}

	return descriptors;
}

}