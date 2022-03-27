#include "IPC.hpp"

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

std::string IPC::Serialize() const
{
  return fmt::format("Rows: {}, Cols: {}, BPL: {}, BPH: {}, L2size: {}, L1ratio: {}, L2Usize: {}, CPeps: {}, BPT: {}, WinT: {}, IntT: {}", GetRows(), GetCols(), GetBandpassL(), GetBandpassH(),
      GetL2size(), GetL1ratio(), GetL2Usize(), GetCrossPowerEpsilon(), BandpassType2String(GetBandpassType()), WindowType2String(GetWindowType()), InterpolationType2String(GetInterpolationType()));
}
