#pragma once

inline cv::Scalar ColormapJet(f32 x, f32 valMin = 0, f32 valMax = 1, f32 val = 255)
{
  f32 B, G, R;
  f32 sh = 0.125 * (valMax - valMin);
  f32 start = valMin;
  f32 mid = valMin + 0.5 * (valMax - valMin);
  f32 end = valMax;

  B = (x > (start + sh)) ? std::clamp(-val / 2 / sh * x + val / 2 / sh * (mid + sh), 0.f, val)
                         : (x < start ? val / 2 : std::clamp(val / 2 / sh * x + val / 2 - val / 2 / sh * start, 0.f, val));
  G = (x < mid) ? std::clamp(val / 2 / sh * x - val / 2 / sh * (start + sh), 0.f, val) : std::clamp(-val / 2 / sh * x + val / 2 / sh * (end - sh), 0.f, val);
  R = (x < (end - sh)) ? std::clamp(val / 2 / sh * x - val / 2 / sh * (mid - sh), 0.f, val)
                       : (x > end ? val / 2 : std::clamp(-val / 2 / sh * x + val / 2 + val / 2 / sh * end, 0.f, val));

  return cv::Scalar(B, G, R);
}

inline cv::ColormapTypes GetColormap(const std::string& cmap)
{
  if (cmap == "gray")
    return cv::COLORMAP_BONE;
  if (cmap == "jet")
    return cv::COLORMAP_JET;
  if (cmap == "viridis")
    return cv::COLORMAP_VIRIDIS;

  return cv::COLORMAP_VIRIDIS;
}
