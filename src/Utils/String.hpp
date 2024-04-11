#pragma once

std::string ExtractSubstring(const std::string& input, std::string_view start, std::string_view end)
{
  size_t startPos = input.find(start);
  if (startPos != std::string::npos)
  {
    startPos += start.length();
    size_t endPos = input.find(end, startPos);
    if (endPos != std::string::npos)
      return input.substr(startPos, endPos - startPos);
  }
  return "";
}
