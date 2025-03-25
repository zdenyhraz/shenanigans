#pragma once

inline cv::Scalar ColormapJet(float x, float valMin = 0, float valMax = 1, float val = 255)
{
  float B, G, R;
  float sh = 0.125 * (valMax - valMin);
  float start = valMin;
  float mid = valMin + 0.5 * (valMax - valMin);
  float end = valMax;

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
