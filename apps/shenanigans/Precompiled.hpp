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
#include <future>

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>
#include <fmt/std.h>
#include <fmt/ranges.h>
#include <fmt/color.h>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <onnxruntime/onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#ifndef PROFILE_FRAME
#  define PROFILE_FRAME
#endif
#ifndef PROFILE_FUNCTION
#  define PROFILE_FUNCTION
#endif
#ifndef PROFILE_SCOPE
#  define PROFILE_SCOPE(name)
#endif

namespace py = pybind11;
namespace json = nlohmann;
using namespace std::complex_literals;
using namespace std::chrono_literals;
using namespace pybind11::literals;

#include "Log/Log.hpp"
