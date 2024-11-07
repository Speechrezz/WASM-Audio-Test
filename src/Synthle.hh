#pragma once
// This file needs to be shared
#include "Array.hh"
#include "AudioProcessor.h"

#include "../external/clap/include/clap/clap.h"
#include "Mutex.hh"

// Forward Declarations
namespace xynth
{
class SynthleGui;
}

struct Voice
{
    bool held;
    int32_t noteID;
    int16_t channel, key;
    float phase;
};
class PluginType
{
    struct Param
    {
        std::string name;
        std::string jsId;
        float minValue;
        float maxValue;
        float defaultValue;
        float step;
        std::string unit;

        // FIXME: Should be private
        struct InternalData
        {
            float value;
            bool changed;
        };

        InternalData audio_thread;
        InternalData gui_thread;
    };

  public:
    // FIXME: These should likely be privatel
    std::vector<Param> m_parameters;
    Mutex m_sync_mtx;

    void addParam(const std::string &name, const std::string &jsId, const float minValue, const float maxValue,
                  const float defaultValue, const float step, const std::string &unit);

    void updateParamFromAudioThread(int id, float value);
    void updateParamFromMainThread(int id, float value);
    bool syncAudioToMain();
    void syncMainToAudio(const clap_output_events_t *out);
};

struct Synthle : public PluginType
{
    constexpr static const clap_plugin_descriptor_t m_descriptor = {
        .clap_version = CLAP_VERSION_INIT,
        .id = "audio.xynth.Synthle",
        .name = "Synthle",
        .vendor = "Xynth Audio",
        .url = "https://xynth.audio",
        .manual_url = "https://xynth.audio",
        .support_url = "https://xynth.audio",
        .version = "0.0.1",
        .description = "THE WORST",

        .features =
            (const char *[]){
                CLAP_PLUGIN_FEATURE_INSTRUMENT,
                CLAP_PLUGIN_FEATURE_SYNTHESIZER,
                CLAP_PLUGIN_FEATURE_STEREO,
                NULL,
            },
    };

    Synthle();
    ~Synthle();

    clap_plugin_t plugin;
    const clap_host_t *host;
    float sampleRate;

    xynth::AudioProcessor audioProcessor;
    xynth::AudioBuffer buffer;

    const clap_host_timer_support_t *hostTimerSupport;
    clap_id timerID;

    xynth::SynthleGui *m_gui;

    Array<Voice> voices;
};
