#pragma once

void addParam(const std::string &name, const std::string &jsId, const float minValue, const float maxValue,
              const float defaultValue, const float step, const std::string &unit);
void init()
{
    addParam("Volume", 0.0, 1.0, 0.5, 0.01, "dB");
}
namespace xynth
{
class SynthlePlugin
{
  public:
};
}; // namespace xynth
