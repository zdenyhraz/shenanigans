#pragma once

// STL
#include <iostream>
#include <fstream>
#include <string>
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

// Qt
#include <QtWidgets>
#include <QtWidgets>
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QWidget>
#include <QCloseEvent>

// OpenMP
#include <omp.h>

// OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
//#include <opencv2/cudaarithm.hpp>
//#include <opencv2/xfeatures2d.hpp>

// QCustomPlot
#include "Plot/qcustomplot.h"

// fmt
#include <fmt/format.h>
#include <fmt/ostream.h>

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

#include "Log/Logger.h"
#include "Log/LogFunction.h"
#include "Core/constants.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"
#include "Draw/showsave.h"
#include "Draw/draw.h"
#include "Draw/crop.h"
