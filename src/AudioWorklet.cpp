
#include <emscripten/val.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Synthle.hh"

#include "AudioProcessor.h"

inline xynth::AudioProcessor audioProcessor;

WebAudioParamDescriptor testParam
{
	.defaultValue = 0.f,
	.minValue = 0.f,
	.maxValue = 1.f,
	.automationRate = WEBAUDIO_PARAM_K_RATE
};

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
	const float volume = *params[0].data;
	xynth::AudioBuffer outputBuffer;
    outputBuffer.resize(outputs);
	audioProcessor.process(outputBuffer, volume);

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

	audioProcessor.prepare(spec);
}

EM_JS(void, linkParameter, (emscripten::val obj, EMSCRIPTEN_AUDIO_WORKLET_NODE_T nodeHandle), {
    // linkParameter(JSON.parse(UTF8ToString(obj)), EmAudio[nodeHandle]);
    linkParameter(JSON.parse(obj), EmAudio[nodeHandle]);
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
	int outputChannelCounts[1] = { getNumberOfChannels() };

	EmscriptenAudioWorkletNodeCreateOptions options = {
		.numberOfInputs = 0,
		.numberOfOutputs = 1,
		.outputChannelCounts = outputChannelCounts};

	// Instantiate the noise-generator Audio Worklet Processor.
	EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "audio-processor", &options, &ProcessAudio, userData);
	// Connect the audio worklet node to the graph.
	emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);

	InitHtmlUi(audioContext);

    Synthle m_plugin;
    // Initialize the parameters in the GUI
    for (int i = 0; i < m_plugin.m_parameters.size(); i++) {
        auto& param = m_plugin.m_parameters[i];
        emscripten::val obj = emscripten::val::object();
        // Set properties
        obj.set("handle", emscripten::val(i));
        obj.set("name", emscripten::val(param.name));
        obj.set("id", emscripten::val(param.jsId));
        obj.set("minValue", emscripten::val(param.minValue));
        obj.set("maxValue", emscripten::val(param.maxValue));
        obj.set("defaultValue", emscripten::val(param.defaultValue));
        obj.set("step", emscripten::val(param.step));
        obj.set("unit", emscripten::val(param.unit));

        emscripten::val json = emscripten::val::global("JSON").call<emscripten::val>("stringify", obj);  // Convert to JSON
        emscripten::val::global("linkParameter").call<void>("call", emscripten::val::global("this"), obj, wasmAudioWorklet);
    }
}

// This callback will fire when the Wasm Module has been shared to the
// AudioWorklet global scope, and is now ready to begin adding Audio Worklet
// Processors.
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData)
{
	if (!success)
		return;

	WebAudioWorkletProcessorCreateOptions opts = {
		.name = "audio-processor",
		.numAudioParams = 1, //audioProcessor.getNumParams(),
		.audioParamDescriptors = &testParam
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

	std::cout << "Browser Sample Rate: " << getSampleRate()
			  << ", numChannels: " << getNumberOfChannels()
			  << ", maximumBlockSize: " << getMaximumBlockSize() << std::endl;

	// Create an audio context
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0 /* use default constructor options */);

	// and kick off Audio Worklet scope initialization, which shares the Wasm
	// Module and Memory to the AudioWorklet scope and initializes its stack.
	emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, nullptr);
}
