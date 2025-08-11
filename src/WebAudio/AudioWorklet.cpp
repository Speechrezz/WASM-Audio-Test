#include <emscripten/bind.h>
#include <emscripten/webaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "AudioProcessorWeb.h"

inline xynth::AudioProcessorWeb audioProcessor;

int parameterIdToIndex(const std::string& id)
{
	return audioProcessor.parameterIdToIndex[id];
}

xynth::AudioParameterView getParameter(const std::string& id)
{
	return { audioProcessor.getProcessor().audioParameters.get(id) };
}

EMSCRIPTEN_BINDINGS(my_module) {
	emscripten::class_<xynth::AudioParameterView>("AudioParameterView")
		.function("getName", &xynth::AudioParameterView::getName)
		.function("getId", &xynth::AudioParameterView::getId)
		.function("convertToNormalizedValue", &xynth::AudioParameterView::convertToNormalizedValue)
		.function("convertFromNormalizedValue", &xynth::AudioParameterView::convertFromNormalizedValue);

    emscripten::function("parameterIdToIndex", &parameterIdToIndex);
    emscripten::function("getParameter", &getParameter);
}

struct UserData
{
	int test = 5;
};

inline UserData userData; // Test

/* Steps to use Wasm-based AudioWorklets:
	1. Create a Web Audio AudioContext either via manual JS code and calling
		emscriptenRegisterAudioObject() from JS, or by calling
		emscripten_create_audio_context() (shown in this sample)
	2. Initialize a Wasm AudioWorklet scope on the audio context by calling
		emscripten_start_wasm_audio_worklet_thread_async(). This shares the Wasm
		Module, Memory, etc. to the AudioWorklet scope, and establishes the stack
		space for the Audio Worklet.
		This needs to be called exactly once during page's lifetime. There is no
		mechanism in Web Audio to shut down/uninitialize the scope.
	3. Create one or more of Audio Worklet Processors with the desired name and
		AudioParam configuration.
	4. Instantiate Web Audio audio graph nodes from the above created worklet
		processors, specifying the desired input-output configurations and Wasm-side
		function callbacks to call for each node.
	5. Add the graph nodes to the Web Audio graph, and the audio callbacks should
		begin to fire.
*/

// This function will be called for every fixed-size buffer of audio samples to be processed.
bool ProcessAudio(int numInputs, const AudioSampleFrame *inputs, 
				  int numOutputs, AudioSampleFrame *outputs, 
				  int numParams, const AudioParamFrame *params, 
				  void *userData)
{
	audioProcessor.updateParameters(numParams, params);
	audioProcessor.process(outputs);

	//int test = static_cast<UserData*>(userData)->test;

	return true; // Return false here to shut down.
}

// Function to get the browser's sample rate
EM_JS(int, getSampleRate, (), {
    return new AudioContext().sampleRate;
});

// Function to get the number of output channels
EM_JS(int, getNumberOfChannels, (), {
    const audioContext = new AudioContext();
    const oscillator = audioContext.createOscillator();
    const numChannels = oscillator.channelCount;
    oscillator.disconnect();
    return numChannels;
});

// Function to get the blockSize;
inline int getMaximumBlockSize()
{
	return 128;
}

void PrepareAudio()
{
	xynth::ProcessSpec spec
	{ 
		.sampleRate = getSampleRate(), 
		.numChannels = getNumberOfChannels(), 
		.maxBlockSize = getMaximumBlockSize()
	};

	std::cout << "Browser Sample Rate: " << spec.sampleRate
			<< ", numChannels: " << spec.numChannels
			<< ", maximumBlockSize: " << spec.maxBlockSize << std::endl;

	audioProcessor.prepare(spec);
}

EM_JS(void, onAudioProcessorInitialized, (EMSCRIPTEN_AUDIO_WORKLET_NODE_T nodeHandle), {
    if (typeof onAudioProcessorInitialized === 'function') {
        onAudioProcessorInitialized(EmAudio[nodeHandle]); // Call the JS function
    }
});

EM_JS(void, InitHtmlUi, (EMSCRIPTEN_WEBAUDIO_T audioContext), {
	// Add a button on the page to toggle playback as a response to user click.
	let startButton = document.createElement('button');
	startButton.innerHTML = 'Toggle playback';
	document.body.appendChild(startButton);

	audioContext = emscriptenGetAudioObject(audioContext);
	startButton.onclick = () =>
	{
		if (audioContext.state != 'running')
		{
			audioContext.resume();
		}
		else
		{
			audioContext.suspend();
		}
	};
});

// This callback will fire after the Audio Worklet Processor has finished being
// added to the Worklet global scope.
void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData)
{
	if (!success)
		return;

	PrepareAudio(); // May need to be called on the AudioWorklet thread, currently being called in the main thread.

	// Specify the input and output node configurations for the Wasm Audio
	// Worklet. A simple setup with single mono output channel here, and no
	// inputs.
	int outputChannelCounts[1] = {getNumberOfChannels()};

	EmscriptenAudioWorkletNodeCreateOptions options = {
		.numberOfInputs = 0,
		.numberOfOutputs = 1,
		.outputChannelCounts = outputChannelCounts};

	// Instantiate the noise-generator Audio Worklet Processor.
	EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "audio-processor", &options, &ProcessAudio, userData);
	// Connect the audio worklet node to the graph.
	emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);

	InitHtmlUi(audioContext);
	onAudioProcessorInitialized(wasmAudioWorklet);
}

// This callback will fire when the Wasm Module has been shared to the
// AudioWorklet global scope, and is now ready to begin adding Audio Worklet
// Processors.
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData)
{
	if (!success)
		return;

	const auto audioParamDescriptors = audioProcessor.getAudioParameterDescriptors();

	WebAudioWorkletProcessorCreateOptions opts = {
		.name = "audio-processor",
		.numAudioParams = (int)audioParamDescriptors.size(),
		.audioParamDescriptors = audioParamDescriptors.data()
	};

	emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, userData);
}

// Define a global stack space for the AudioWorkletGlobalScope. Note that all
// AudioWorkletProcessors and/or AudioWorkletNodes on the given Audio Context
// all share the same AudioWorkerGlobalScope, i.e. they all run on the same one
// audio thread (multiple nodes/processors do not each get their own thread).
// Hence one stack is enough.
uint8_t wasmAudioWorkletStack[4096];

int main()
{
	srand(time(NULL));

	assert(!emscripten_current_thread_is_audio_worklet());

	// Create an audio context
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0 /* use default constructor options */);
    
	// and kick off Audio Worklet scope initialization, which shares the Wasm
	// Module and Memory to the AudioWorklet scope and initializes its stack.
	emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, &userData);
}