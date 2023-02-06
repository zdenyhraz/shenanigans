#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <chrono>
#include <numeric>
#include <filesystem>
#include <queue>
#include <functional>
#include <vector>
#include <complex>
#include <tuple>
#include <thread>
#include <memory>
#include <regex>
#include <ctime>
#include <limits>
#include <random>
#include <algorithm>
#include <mutex>
#include <map>
#include <unordered_map>
#include <stdexcept>
#include <cstdint>
#include <variant>
#include <initializer_list>
#include <span>
#include <type_traits>
#include <concepts>
#include <source_location>
#include <numbers>
#include <bit>
#include <ranges>

#define ImDrawIdx unsigned int
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <implot.h>
#include <implot_internal.h>

#include <omp.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <torch/torch.h>
#include <benchmark/benchmark.h>

#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
// #include <opencv2/core/cuda.hpp>
// #include <opencv2/cudaarithm.hpp>

#ifdef ENABLE_PROFILING
  #include <Tracy.hpp>
  // use FrameMark for frames (at the end of each frame)
  // use ZoneScoped once per scope (automatic name)
  // use ZoneScopedN once per scope (user-supplied name)
  // use ZoneNamedN for scopes inside ZoneScoped scope (user-supplied name)
  // bool parameter of ZoneNamed can turn it on/off
  #define PROFILE_FRAME FrameMark
  #define PROFILE_FUNCTION ZoneScoped
  #define PROFILE_SCOPE(name) ZoneNamedN(name, #name, true)

inline void* operator new(std::size_t count)
{
  auto ptr = malloc(count);
  TracyAlloc(ptr, count);
  return ptr;
}

inline void operator delete(void* ptr) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

#else
  #define PROFILE_FRAME
  #define PROFILE_FUNCTION
  #define PROFILE_SCOPE(name)
#endif

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using usize = size_t;
using f32 = float;
using f64 = double;
using f128 = long double;

namespace py = pybind11;
namespace json = nlohmann;
using namespace std::complex_literals;
using namespace std::chrono_literals;
using namespace pybind11::literals;

#include "Log/Log.hpp"
#include "Log/LogFunction.hpp"

#include "Math/Math.hpp"
#include "Math/Functions.hpp"
#include "Math/Transform.hpp"
#include "Math/Statistics.hpp"

#include "Utils/DateTime.hpp"
#include "Utils/Operators.hpp"
#include "Utils/Filesystem.hpp"
#include "Utils/Timer.hpp"
#include "Utils/Formatters.hpp"
#include "Utils/Random.hpp"
#include "Utils/ThreadUtils.hpp"
#include "Utils/Crop.hpp"
#include "Utils/Draw.hpp"
#include "Utils/Load.hpp"
#include "Utils/Vectmat.hpp"

#include "Plot/Plot.hpp"
#include "Plot/PyPlot.hpp"
