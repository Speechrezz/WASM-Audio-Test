#include "../external/clap/include/clap/clap.h"
#include "AudioProcessor.h"

#include "CLAP/Gui.hh"
#include "Synthle.hh"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
static std::ofstream clap_debug_out("/Users/Joe/Documents/debug_out.txt");

static const clap_plugin_note_ports_t extensionNotePorts = {
    .count = [](const clap_plugin_t *plugin, bool isInput) -> uint32_t { return isInput ? 1 : 0; },

    .get = [](const clap_plugin_t *plugin, uint32_t index, bool isInput, clap_note_port_info_t *info) -> bool {
        if (!isInput || index)
            return false;
        info->id = 0;
        info->supported_dialects = CLAP_NOTE_DIALECT_CLAP; // TODO Also support the MIDI dialect.
        info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
        snprintf(info->name, sizeof(info->name), "%s", "Note Port");
        return true;
    },
};

static const clap_plugin_audio_ports_t extensionAudioPorts = {
    .count = [](const clap_plugin_t *plugin, bool isInput) -> uint32_t {
        if (isInput)
            return 0;
        return 3;
    },

    .get = [](const clap_plugin_t *plugin, uint32_t index, bool isInput, clap_audio_port_info_t *info) -> bool {
        if (isInput || index)
            return false;
        info->id = 0;
        info->channel_count = 2;
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = CLAP_INVALID_ID;
        snprintf(info->name, sizeof(info->name), "%s", "Audio Output");
        return true;
    },
};

static void PluginProcessEvent(Synthle *plugin, const clap_event_header_t *event)
{
    if (event->space_id == CLAP_CORE_EVENT_SPACE_ID)
    {
        if (event->type == CLAP_EVENT_NOTE_ON || event->type == CLAP_EVENT_NOTE_OFF ||
            event->type == CLAP_EVENT_NOTE_CHOKE)
        {
            const clap_event_note_t *noteEvent = (const clap_event_note_t *)event;

            // Look through our voices array, and if the event matches any of them,
            // it must have been released.
            for (int i = 0; i < plugin->voices.Length(); i++)
            {
                Voice *voice = &plugin->voices[i];

                if ((noteEvent->key == -1 || voice->key == noteEvent->key) &&
                    (noteEvent->note_id == -1 || voice->noteID == noteEvent->note_id) &&
                    (noteEvent->channel == -1 || voice->channel == noteEvent->channel))
                {
                    if (event->type == CLAP_EVENT_NOTE_CHOKE)
                    {
                        plugin->voices.Delete(i--); // Stop the voice immediately; don't process the release segment of
                                                    // any ADSR envelopes.
                    }
                    else
                    {
                        voice->held = false;
                    }
                }
            }

            // If this is a note on event, create a new voice and add it to our array.
            if (event->type == CLAP_EVENT_NOTE_ON)
            {
                Voice voice = {
                    .held = true,
                    .noteID = noteEvent->note_id,
                    .channel = noteEvent->channel,
                    .key = noteEvent->key,
                    .phase = 0.0f,
                };

                plugin->voices.Add(voice);
            }
        }
        if (event->type == CLAP_EVENT_PARAM_VALUE)
        {
            const clap_event_param_value_t *valueEvent = (const clap_event_param_value_t *)event;
            uint32_t i = (uint32_t)valueEvent->param_id;
            MutexAcquire(plugin->syncParameters);
            plugin->m_parameters[i].value = valueEvent->value;
            plugin->m_parameters[i].changed = true;
            MutexRelease(plugin->syncParameters);
        }
        if (event->type == CLAP_EVENT_PARAM_MOD)
        {
            const clap_event_param_mod_t *modEvent = (const clap_event_param_mod_t *)event;

            for (int i = 0; i < plugin->voices.Length(); i++)
            {
                Voice *voice = &plugin->voices[i];

                if ((modEvent->key == -1 || voice->key == modEvent->key) &&
                    (modEvent->note_id == -1 || voice->noteID == modEvent->note_id) &&
                    (modEvent->channel == -1 || voice->channel == modEvent->channel))
                {
                    // voice->parameterOffsets[modEvent->param_id] = modEvent->amount;
                    break;
                }
            }
        }
    }
}

static const clap_plugin_params_t extensionParams = {
    .count = [](const clap_plugin_t *_plugin) -> uint32_t {
        auto plugin = (Synthle *)_plugin->plugin_data;
        return plugin->m_real_parameters.size();
    },

    .get_info = [](const clap_plugin_t *_plugin, uint32_t index, clap_param_info_t *information) -> bool {
        if (index == Parameters::Volume)
        {
            memset(information, 0, sizeof(clap_param_info_t));
            information->id = index;
            // These flags enable polyphonic modulation.
            information->flags =
                CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_MODULATABLE | CLAP_PARAM_IS_MODULATABLE_PER_NOTE_ID;
            information->min_value = 0.0f;
            information->max_value = 1.0f;
            information->default_value = 0.5f;
            strcpy(information->name, "Volume");
            return true;
        }
        else
        {
            return false;
        }
    },

    .get_value = [](const clap_plugin_t *_plugin, clap_id id, double *value) -> bool {
        Synthle *plugin = (Synthle *)_plugin->plugin_data;
        uint32_t i = (uint32_t)id;
        if (i >= plugin->m_real_parameters.size())
            return false;

        // get_value is called on the main thread, but should return the value of the parameter according to the audio
        // thread, since the value on the audio thread is the one that host communicates with us via
        // CLAP_EVENT_PARAM_VALUE events. Since we're accessing the opposite thread's arrays, we must acquire the
        // syncParameters mutex. And although we need to check the mainChanged array, we mustn't actually modify the
        // parameters array, since that can only be done on the audio thread. Don't worry -- it'll pick up the changes
        // eventually.

        MutexAcquire(plugin->syncParameters);
        *value = plugin->m_parameters_main_thread[i].changed ? plugin->m_parameters_main_thread[i].value
                                                             : plugin->m_parameters[i].value;
        MutexRelease(plugin->syncParameters);
        return true;
    },

    .value_to_text =
        [](const clap_plugin_t *_plugin, clap_id id, double value, char *display, uint32_t size) {
            auto plugin = (Synthle *)_plugin->plugin_data;
            uint32_t i = (uint32_t)id;
            if (i >= plugin->m_real_parameters.size())
                return false;
            snprintf(display, size, "%f", value);
            return true;
        },

    .text_to_value =
        [](const clap_plugin_t *_plugin, clap_id param_id, const char *display, double *value) {
            // TODO Implement this.
            return false;
        },

    .flush =
        [](const clap_plugin_t *_plugin, const clap_input_events_t *in, const clap_output_events_t *out) {
            Synthle *plugin = (Synthle *)_plugin->plugin_data;
            const uint32_t eventCount = in->size(in);

            // For parameters that have been modified by the main thread, send CLAP_EVENT_PARAM_VALUE events to the
            // host.
            plugin->syncMainToAudio(out);

            // Process events sent to our plugin from the host.
            for (uint32_t eventIndex = 0; eventIndex < eventCount; eventIndex++)
            {
                PluginProcessEvent(plugin, in->get(in, eventIndex));
            }
        },
};

static void PluginRenderAudio(Synthle *plugin, uint32_t start, uint32_t end, float *outputL, float *outputR)
{
    for (uint32_t index = start; index < end; index++)
    {
        float sum = 0.0f;

        for (int i = 0; i < plugin->voices.Length(); i++)
        {
            Voice *voice = &plugin->voices[i];
            if (!voice->held)
                continue;
            sum += sinf(voice->phase * 2.0f * 3.14159f) * 0.2f;
            voice->phase += 440.0f * exp2f((voice->key - 57.0f) / 12.0f) / plugin->sampleRate;
            voice->phase -= floorf(voice->phase);
        }

        outputL[index] = sum;
        outputR[index] = sum;
    }
}

static const clap_plugin_gui_t extensionGUI = {
    .is_api_supported = [](const clap_plugin_t *plugin, const char *api, bool isFloating) -> bool {
        // We'll define GUI_API in our platform specific code file.
        return 0 == strcmp(api, GUI_API) && !isFloating;
    },

    .get_preferred_api = [](const clap_plugin_t *plugin, const char **api, bool *isFloating) -> bool {
        *api = GUI_API;
        *isFloating = false;
        return true;
    },

    .create = [](const clap_plugin_t *_plugin, const char *api, bool isFloating) -> bool {
        if (!extensionGUI.is_api_supported(_plugin, api, isFloating))
            return false;
        auto plugin = (Synthle *)_plugin->plugin_data;
        // We'll define GUICreate in our platform specific code file.
        plugin->m_gui = new xynth::SynthleGui(plugin);
        return true;
    },

    .destroy =
        [](const clap_plugin_t *_plugin) {
            auto plugin = (Synthle *)_plugin->plugin_data;
            delete plugin->m_gui;
        },

    .set_scale = [](const clap_plugin_t *plugin, double scale) -> bool { return false; },

    .get_size = [](const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) -> bool {
        *width = GUI_WIDTH;
        *height = GUI_HEIGHT;
        return true;
    },

    .can_resize = [](const clap_plugin_t *plugin) -> bool { return false; },

    .get_resize_hints = [](const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) -> bool { return false; },

    .adjust_size = [](const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) -> bool {
        return extensionGUI.get_size(plugin, width, height);
    },

    .set_size = [](const clap_plugin_t *plugin, uint32_t width, uint32_t height) -> bool { return true; },

    .set_parent = [](const clap_plugin_t *_plugin, const clap_window_t *window) -> bool {
        assert(0 == strcmp(window->api, GUI_API));
        auto plugin = (Synthle *)_plugin->plugin_data;
#ifdef __APPLE__
        plugin->m_gui->setParent(window->cocoa);
#endif
        return true;
    },

    .set_transient = [](const clap_plugin_t *plugin, const clap_window_t *window) -> bool { return false; },

    .suggest_title = [](const clap_plugin_t *plugin, const char *title) {},

    .show = [](const clap_plugin_t *_plugin) -> bool {
        auto plugin = (Synthle *)_plugin->plugin_data;
        plugin->m_gui->setVisible(true);
        return true;
    },

    .hide = [](const clap_plugin_t *_plugin) -> bool {
        auto plugin = (Synthle *)_plugin->plugin_data;
        plugin->m_gui->setVisible(false);
        return true;
    },
};

static const clap_plugin_timer_support_t extensionTimerSupport = {
    .on_timer =
        [](const clap_plugin_t *_plugin, clap_id timerID) {
            Synthle *plugin = (Synthle *)_plugin->plugin_data;

            if (plugin->m_gui && plugin->syncAudioToMain())
            {
                plugin->m_gui->paint();
            }
        },
};

static const clap_plugin_t pluginClass = {
    .desc = &Synthle::m_descriptor,
    .plugin_data = nullptr,

    .init = [](const clap_plugin *_plugin) -> bool {
        Synthle *plugin = (Synthle *)_plugin->plugin_data;

        plugin->hostTimerSupport =
            (const clap_host_timer_support_t *)plugin->host->get_extension(plugin->host, CLAP_EXT_TIMER_SUPPORT);
        std::cout << "host timer support? " << plugin->hostTimerSupport << std::endl;
        MutexInitialise(plugin->syncParameters);

        for (uint32_t i = 0; i < plugin->m_real_parameters.size(); i++)
        {
            clap_param_info_t information = {};
            extensionParams.get_info(_plugin, i, &information);
            plugin->m_parameters_main_thread[i].value = plugin->m_parameters[i].value = information.default_value;
        }

        if (plugin->hostTimerSupport && plugin->hostTimerSupport->register_timer)
        {
            plugin->hostTimerSupport->register_timer(plugin->host, 200, &plugin->timerID);
        }

        (void)plugin;
        return true;
    },

    .destroy =
        [](const clap_plugin *_plugin) {
            Synthle *plugin = (Synthle *)_plugin->plugin_data;
            plugin->voices.Free();
            MutexDestroy(plugin->syncParameters);

            if (plugin->hostTimerSupport && plugin->hostTimerSupport->register_timer)
            {
                plugin->hostTimerSupport->unregister_timer(plugin->host, plugin->timerID);
            }
            free(plugin);
        },

    .activate = [](const clap_plugin *_plugin, double sampleRate, uint32_t minimumFramesCount,
                   uint32_t maximumFramesCount) -> bool {
        Synthle *plugin = (Synthle *)_plugin->plugin_data;
        plugin->sampleRate = sampleRate;
        return true;
    },

    .deactivate = [](const clap_plugin *_plugin) {},

    .start_processing = [](const clap_plugin *_plugin) -> bool {
        Synthle *plugin = (Synthle *)_plugin->plugin_data;
        xynth::ProcessSpec spec;
        spec.sampleRate = plugin->sampleRate;
        spec.numChannels = 2;     // FIXME:
        spec.maxBlockSize = 8192; // FIXME:
        plugin->audioProcessor.prepare(spec);

        return true;
    },

    .stop_processing =
        [](const clap_plugin *_plugin) {
            // FIXME:
        },

    .reset =
        [](const clap_plugin *_plugin) {
            Synthle *plugin = (Synthle *)_plugin->plugin_data;
            // FIXME: plugin->voices.Free();
        },

    .process = [](const clap_plugin *_plugin, const clap_process_t *process) -> clap_process_status {
        Synthle *plugin = (Synthle *)_plugin->plugin_data;

        plugin->syncMainToAudio(process->out_events);

        assert(process->audio_outputs_count == 1);
        assert(process->audio_inputs_count == 0);

        const uint32_t frameCount = process->frames_count;
        const uint32_t inputEventCount = process->in_events->size(process->in_events);
        uint32_t eventIndex = 0;
        uint32_t nextEventFrame = inputEventCount ? 0 : frameCount;

        for (uint32_t i = 0; i < frameCount;)
        {
            while (eventIndex < inputEventCount && nextEventFrame == i)
            {
                const clap_event_header_t *event = process->in_events->get(process->in_events, eventIndex);

                if (event->time != i)
                {
                    nextEventFrame = event->time;
                    break;
                }

                PluginProcessEvent(plugin, event);
                eventIndex++;

                if (eventIndex == inputEventCount)
                {
                    nextEventFrame = frameCount;
                    break;
                }
            }

            PluginRenderAudio(plugin, i, nextEventFrame, process->audio_outputs[0].data32[0],
                              process->audio_outputs[0].data32[1]);
            i = nextEventFrame;
        }

        // const float volume = *params[0].data;

        plugin->buffer.resize(process);
        plugin->audioProcessor.process(plugin->buffer, plugin->m_parameters[0].value);

        for (int i = 0; i < plugin->voices.Length(); i++)
        {
            Voice *voice = &plugin->voices[i];

            if (!voice->held)
            {
                clap_event_note_t event = {};
                event.header.size = sizeof(event);
                event.header.time = 0;
                event.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
                event.header.type = CLAP_EVENT_NOTE_END;
                event.header.flags = 0;
                event.key = voice->key;
                event.note_id = voice->noteID;
                event.channel = voice->channel;
                event.port_index = 0;
                process->out_events->try_push(process->out_events, &event.header);

                plugin->voices.Delete(i--);
            }
        }

        return CLAP_PROCESS_CONTINUE;
    },

    .get_extension = [](const clap_plugin *plugin, const char *id) -> const void * {
        if (0 == strcmp(id, CLAP_EXT_NOTE_PORTS))
            return &extensionNotePorts;
        if (0 == strcmp(id, CLAP_EXT_AUDIO_PORTS))
            return &extensionAudioPorts;
        if (0 == strcmp(id, CLAP_EXT_PARAMS))
            return &extensionParams;
        if (0 == strcmp(id, CLAP_EXT_GUI))
            return &extensionGUI;
        if (0 == strcmp(id, CLAP_EXT_TIMER_SUPPORT))
            return &extensionTimerSupport;
        return nullptr;
    },

    .on_main_thread = [](const clap_plugin *_plugin) {},
};

static const clap_plugin_factory_t pluginFactory = {
    .get_plugin_count = [](const clap_plugin_factory *factory) -> uint32_t { return 1; },

    .get_plugin_descriptor = [](const clap_plugin_factory *factory,
                                uint32_t index) -> const clap_plugin_descriptor_t * {
        // Return a pointer to our pluginDescriptor definition.
        return index == 0 ? &Synthle::m_descriptor : nullptr;
    },

    .create_plugin = [](const clap_plugin_factory *factory, const clap_host_t *host,
                        const char *pluginID) -> const clap_plugin_t * {
        if (!clap_version_is_compatible(host->clap_version) || strcmp(pluginID, Synthle::m_descriptor.id))
        {
            return nullptr;
        }
        std::cout.rdbuf(clap_debug_out.rdbuf());
        // Allocate the plugin structure, and fill in the plugin information from
        // the pluginClass variable.
        Synthle *plugin = new Synthle;
        plugin->host = host;
        plugin->plugin = pluginClass;
        plugin->plugin.plugin_data = plugin;
        return &plugin->plugin;
    },
};

extern "C" const clap_plugin_entry_t clap_entry = {
    .clap_version = CLAP_VERSION_INIT,

    .init = [](const char *path) -> bool { return true; },

    .deinit = []() {},

    .get_factory = [](const char *factoryID) -> const void * {
        // Return a pointer to our pluginFactory definition.
        return strcmp(factoryID, CLAP_PLUGIN_FACTORY_ID) ? nullptr : &pluginFactory;
    },
};
