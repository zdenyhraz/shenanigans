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
#include <initializer_list>
#include <span>
#include <type_traits>
#include <concepts>

#include <omp.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <torch/torch.h>

#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include <QtPrintSupport>
#include <qcustomplot.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
//#include <opencv2/core/cuda.hpp>
//#include <opencv2/cudaarithm.hpp>

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
using f32 = float;
using f64 = double;
using f128 = long double;
using usize = size_t;

namespace py = pybind11;
namespace json = nlohmann;

#include "Log/Logger.hpp"
#include "Log/LogFunction.hpp"

#include "Utils/Constants.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Timer.hpp"
#include "Utils/Random.hpp"
#include "Utils/Formatters.hpp"

#include "UtilsCV/Utils.hpp"
#include "UtilsCV/Crop.hpp"
#include "UtilsCV/Draw.hpp"
#include "UtilsCV/Showsave.hpp"
#include "UtilsCV/Formatters.hpp"

#include "Plot/Plot1D.hpp"
#include "Plot/Plot2D.hpp"
#include "Plot/PyPlot.hpp"
