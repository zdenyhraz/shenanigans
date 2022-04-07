#pragma once

#define PYTHON_INTERPRETER_GUARD std::scoped_lock lock(Python::GetMutex())

class Python
{
  using MutexType = std::recursive_mutex;
  inline static MutexType mMutex;

public:
  static void Initialize()
  {
    PROFILE_FUNCTION;
    PYTHON_INTERPRETER_GUARD;
    LOG_DEBUG("Initializing Python ...");
    static py::scoped_interpreter guard{};
    const auto projectDirectory = std::filesystem::current_path().parent_path();
    py::exec(fmt::format("import sys\r\nsys.path.append('{}')", (projectDirectory / "script").string())); // for Python module importing by C++
    py::exec(fmt::format("import sys\r\nsys.path.append('{}')", (projectDirectory / "build").string()));  // for C++ module importing by Python
  }

  static MutexType& GetMutex() { return mMutex; }

  template <typename T>
  static py::array_t<T> ToNumpy(const std::vector<T>& vec)
  {
    return py::array(vec.size(), vec.data());
  }

  template <typename T>
  static py::array_t<T> ToNumpy(const cv::Mat& mat)
  {
    return py::array_t<T>({mat.rows, mat.cols}, reinterpret_cast<T*>(mat.data));
  }
};
