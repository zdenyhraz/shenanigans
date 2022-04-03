#pragma once

class Python
{
public:
  static void Initialize()
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("Python::Initialize()");
    LOG_DEBUG("Initializing Python ...");
    static py::scoped_interpreter guard{};
  }
};
