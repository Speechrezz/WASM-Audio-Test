#include "Synthle.hh"

void Synthle::addParam(const std::string &name, const std::string &jsId, const float minValue, const float maxValue,
                       const float defaultValue, const float step, const std::string &unit)
{
    m_parameters.push_back({defaultValue, false});
    m_parameters_main_thread.push_back({defaultValue, false});
    m_real_parameters.push_back({name, jsId, minValue, maxValue, defaultValue, step, unit});
}

Synthle::Synthle()
{
    addParam("Volume", "volume", 0, 1, 0, 0.01, "dB");
}

Synthle::~Synthle()
{
}
