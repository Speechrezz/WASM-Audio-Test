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

struct Synthle
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

    void addParam(const std::string &name, const std::string &jsId, const float minValue, const float maxValue,
                  const float defaultValue, const float step, const std::string &unit);

    Synthle();
    ~Synthle();

    clap_plugin_t plugin;
    const clap_host_t *host;
    float sampleRate;

    xynth::AudioProcessor audioProcessor;
    xynth::AudioBuffer buffer;

    const clap_host_timer_support_t *hostTimerSupport;
    clap_id timerID;

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

    std::vector<Param> m_parameters;

    // If the audio thread updates parameters it will update this array.
    inline void updateParamAudioThread(int id, float value)
    {
        m_parameters[id].audio_thread = {value, true};
    }

    // If the main (GUI) thread updates parameters it will update this array.
    inline void updateParamMainThread(int id, float value)
    {
        m_parameters[id].gui_thread = {value, true};
    }

    bool syncAudioToMain()
    {
        bool any_changed = false;
        MutexAcquire(syncParameters);

        for (uint32_t i = 0; i < m_parameters.size(); i++)
        {
            if (m_parameters[i].audio_thread.changed)
            {
                m_parameters[i].gui_thread.value = m_parameters[i].audio_thread.value;
                m_parameters[i].audio_thread.changed = false;
                any_changed = true;
            }
        }

        MutexRelease(syncParameters);
        return any_changed;
    }

    void syncMainToAudio(const clap_output_events_t *out)
    {
        MutexAcquire(syncParameters);

        for (uint32_t i = 0; i < m_parameters.size(); i++)
        {
            if (m_parameters[i].gui_thread.changed)
            {
                m_parameters[i].audio_thread.value = m_parameters[i].gui_thread.value;
                m_parameters[i].gui_thread.changed = false;

                clap_event_param_value_t event = {};
                event.header.size = sizeof(event);
                event.header.time = 0;
                event.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
                event.header.type = CLAP_EVENT_PARAM_VALUE;
                event.header.flags = 0;
                event.param_id = i;
                event.cookie = NULL;
                event.note_id = -1;
                event.port_index = -1;
                event.channel = -1;
                event.key = -1;
                event.value = m_parameters[i].audio_thread.value;
                out->try_push(out, &event.header);
            }
        }

        MutexRelease(syncParameters);
    }

    Mutex syncParameters;

    xynth::SynthleGui *m_gui;

    Array<Voice> voices;
};
