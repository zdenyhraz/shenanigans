#pragma once

inline void PythonInitialize()
{
  LOG_DEBUG("Initializing Python ...");
  static py::scoped_interpreter guard{};
}
