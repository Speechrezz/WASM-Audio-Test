#include "Synthle.hh"

void PluginType::addParam(const std::string &name, const std::string &jsId, const float minValue, const float maxValue,
                          const float defaultValue, const float step, const std::string &unit)
{
    assert(jsId[0] == '#' && "The JavaScript element ID must begin with a '#' symbol");
    m_parameters.push_back({
        name,
        jsId,
        minValue,
        maxValue,
        defaultValue,
        step,
        unit,
        {defaultValue, false},
        {defaultValue, false},
    });
    std::cout << "Added param with name:" << name;
}

void PluginType::updateParamFromAudioThread(int id, float value)
{
    m_parameters[id].audio_thread = {value, true};
}
void PluginType::updateParamFromMainThread(int id, float value)
{
    m_parameters[id].gui_thread = {value, true};
}
bool PluginType::syncAudioToMain()
{
    bool any_changed = false;
    MutexAcquire(m_sync_mtx);

    for (uint32_t i = 0; i < m_parameters.size(); i++)
    {
        if (m_parameters[i].audio_thread.changed)
        {
            m_parameters[i].gui_thread.value = m_parameters[i].audio_thread.value;
            m_parameters[i].audio_thread.changed = false;
            any_changed = true;
        }
    }

    MutexRelease(m_sync_mtx);
    return any_changed;
}

void PluginType::syncMainToAudio(const clap_output_events_t *out)
{
    MutexAcquire(m_sync_mtx);

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

    MutexRelease(m_sync_mtx);
}

Synthle::Synthle()
{
    addParam("Volume", "#volume", 0, 1, 0, 0.01, "%");
    addParam("Volume Again", "#volume2", 0, 1, 0, 0.01, "dB");
}

Synthle::~Synthle()
{
}
