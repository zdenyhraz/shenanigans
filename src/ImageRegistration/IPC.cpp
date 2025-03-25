#include "IPC.hpp"
#include "Plot/Plot.hpp"

std::string IPC::BandpassType2String(BandpassType type)
{
  switch (type)
  {
  case BandpassType::Rectangular:
    return "Rect";
  case BandpassType::Gaussian:
    return "Gauss";
  case BandpassType::None:
    return "None";
  default:
    return "Unknown";
  }
}

std::string IPC::WindowType2String(WindowType type)
{
  switch (type)
  {
  case WindowType::None:
    return "None";
  case WindowType::Hann:
    return "Hann";
  default:
    return "Unknown";
  }
}

std::string IPC::L1WindowType2String(L1WindowType type)
{
  switch (type)
  {
  case L1WindowType::None:
    return "None";
  case L1WindowType::Circular:
    return "Circular";
  case L1WindowType::Gaussian:
    return "Gaussian";
  default:
    return "Unknown";
  }
}

std::string IPC::InterpolationType2String(InterpolationType type)
{
  switch (type)
  {
  case InterpolationType::NearestNeighbor:
    return "NN";
  case InterpolationType::Linear:
    return "Linear";
  case InterpolationType::Cubic:
    return "Cubic";
  default:
    return "Unknown";
  }
}

// cppcheck-suppress unusedFunction
std::string IPC::Serialize() const
{
  return fmt::format("Rows: {}, Cols: {}, BPL: {}, BPH: {}, L2size: {}, L1ratio: {}, L2Usize: {}, CPeps: {}, BPT: {}, WinT: {}, IntT: {}", GetRows(), GetCols(), GetBandpassL(),
      GetBandpassH(), GetL2size(), GetL1ratio(), GetL2Usize(), GetCrossPowerEpsilon(), BandpassType2String(GetBandpassType()), WindowType2String(GetWindowType()),
      InterpolationType2String(GetInterpolationType()));
}

void IPC::FalseCorrelationsRemoval(cv::Mat& L3) const
{
  const auto radius = 5;
  const auto kirkl = 1. - Kirkl<Float>(L3.rows, L3.cols, radius);
  Plot::Plot("FCR L3 raw", L3);
  Plot::Plot({.name = "L2U raw", .z = CalculateL2U(CalculateL2(L3, cv::Point2d(L3.cols / 2, L3.rows / 2), 39)), .surf = true});
  cv::multiply(L3, kirkl, L3);
  Plot::Plot({.name = "L2U fcr", .z = CalculateL2U(CalculateL2(L3, cv::Point2d(L3.cols / 2, L3.rows / 2), 39)), .surf = true});
  throw std::runtime_error("stap xd");
}
