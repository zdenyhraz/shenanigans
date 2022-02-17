#pragma once

// STL
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

// OpenMP
#include <omp.h>

// pybind11
#include <pybind11/embed.h>
#include <pybind11/stl.h>

// Qt
#include <QtWidgets>
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QWidget>
#include <QCloseEvent>
#include <QTextBrowser>
#include <QCoreApplication>

// QCustomPlot
#include <qcustomplot.h>

// OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
//#include <opencv2/core/cuda.hpp>
//#include <opencv2/cudaarithm.hpp>

// fmt
#include <fmt/format.h>
#include <fmt/ostream.h>

// gtest
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// json
#include <nlohmann/json.hpp>

// custom
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
namespace json = nlohmann;
namespace py = pybind11;

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

#include "Logger/Logger.h"
#include "Logger/LogFunction.h"

#include "Utils/Constants.h"
#include "Utils/Utils.h"
#include "Utils/Timer.h"
#include "Utils/Formatters.h"

#include "UtilsCV/Utils.h"
#include "UtilsCV/Crop.h"
#include "UtilsCV/Draw.h"
#include "UtilsCV/Showsave.h"
#include "UtilsCV/Formatters.h"

#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"
#include "Plot/PyPlot.h"
