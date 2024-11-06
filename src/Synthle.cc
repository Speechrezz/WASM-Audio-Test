#include "Synthle.hh"

void Synthle::addParam(const std::string &name, const std::string &jsId, const float minValue, const float maxValue,
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
}

Synthle::Synthle()
{
    addParam("Volume", "#volume", 0, 1, 0, 0.01, "%");
    addParam("Volume Again", "#volume2", 0, 1, 0, 0.01, "dB");
}

Synthle::~Synthle()
{
}
