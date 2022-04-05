#pragma once

class Python
{
public:
  static void Initialize()
  {
    PROFILE_FUNCTION;
    LOG_DEBUG("Initializing Python ...");
    static py::scoped_interpreter guard{};
    const auto projectDirectory = std::filesystem::current_path().parent_path();
    py::exec(fmt::format("import sys\r\nsys.path.append('{}')", (projectDirectory / "script").string())); // for Python module importing by C++
    py::exec(fmt::format("import sys\r\nsys.path.append('{}')", (projectDirectory / "build").string()));  // for C++ module importing by Python
  }
};
