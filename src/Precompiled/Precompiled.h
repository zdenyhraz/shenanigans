#pragma once

#include "PrecompiledSTL.h"
#include "PrecompiledExternal.h"
#include "PrecompiledQt.h"
#include "PrecompiledCV.h"

namespace py = pybind11;
namespace json = nlohmann;

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
