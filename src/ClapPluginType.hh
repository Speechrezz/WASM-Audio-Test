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

enum Parameters
{
    Volume = 0
};
struct Voice
{
    bool held;
    int32_t noteID;
    int16_t channel, key;
    float phase;
};

struct MyPlugin
{
    static constexpr int kNParams = 1;
    clap_plugin_t plugin;
    const clap_host_t *host;
    float sampleRate;

    xynth::AudioProcessor audioProcessor;
    xynth::AudioBuffer buffer;

    const clap_host_timer_support_t *hostTimerSupport;
    clap_id timerID;

    // If the audio thread updates parameters it will update this array.
    float m_parameters[kNParams];
    bool m_parameters_changed[kNParams];
    inline void updateParamAudioThread(int id, float value)
    {
        m_parameters[id] = value;
        m_parameters_changed[id] = true;
    }

    // If the main (GUI) thread updates parameters it will update this array.
    float m_parameters_main_thread[kNParams];
    bool m_parameters_main_thread_changed[kNParams];
    inline void updateParamMainThread(int id, float value)
    {
        std::cout << "ID: " << id << " Value: " << value << std::endl;
        m_parameters_main_thread[id] = value;
        m_parameters_main_thread_changed[id] = true;
    }

    bool syncAudioToMain()
    {
        bool any_changed = false;
        MutexAcquire(syncParameters);

        for (uint32_t i = 0; i < MyPlugin::kNParams; i++)
        {
            if (m_parameters_changed[i])
            {
                m_parameters_main_thread[i] = m_parameters[i];
                m_parameters_changed[i] = false;
                any_changed = true;
            }
        }

        MutexRelease(syncParameters);
        return any_changed;
    }

    void syncMainToAudio(const clap_output_events_t *out)
    {
        MutexAcquire(syncParameters);

        for (uint32_t i = 0; i < MyPlugin::kNParams; i++)
        {
            if (m_parameters_main_thread_changed[i])
            {
                m_parameters[i] = m_parameters_main_thread[i];
                m_parameters_main_thread_changed[i] = false;

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
                event.value = m_parameters[i];
                out->try_push(out, &event.header);
            }
        }

        MutexRelease(syncParameters);
    }

    Mutex syncParameters;

    xynth::SynthleGui *m_gui;

    Array<Voice> voices;
};
