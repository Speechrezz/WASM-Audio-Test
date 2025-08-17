#include "ParameterText.h"
#include <iomanip>
#include <sstream>
#include <cstdlib>

namespace xynth
{
    
std::string floatToString(float value, int maxLength)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(maxLength) << value;
    return out.str();
}

std::string frequencyAsText(float value, int maxLength)
{
    if (value >= 1000.f) 
    {
        value = value / 1000.f;
        return floatToString(value, maxLength) + " kHz";
    }

    return floatToString(value, std::max(maxLength - 1, 0)) + " Hz";
}

std::string msAsText(float value, int maxLength)
{
    if (value >= 1000.f)
    {
        value /= 1000.f;
        return floatToString(value, maxLength) + " s";
    }

    return floatToString(value, maxLength) + " ms";
}

float textToValue(const std::string& text)
{
    float value = std::atof(text.c_str());
    if (text.find("k") != std::string::npos || text.find("K") != std::string::npos)
        value *= 1000.f;

    return value;
}

}