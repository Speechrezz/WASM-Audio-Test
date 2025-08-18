#pragma once

#include <string>

namespace xylo
{

std::string floatToString(float value, int maxLength = 2);
std::string frequencyAsText(float value, int maxLength = 2);
std::string msAsText(float value, int maxLength = 2);

float textToValue(const std::string& text);

}