#pragma once
#include "opencv2/opencv.hpp"
#include "Core/functionsBaseSTL.h"

inline cv::Mat roicrop(const cv::Mat& sourceimgIn, int x, int y, int w, int h)
{
  if (x < 0 || y < 0 || x - w / 2 < 0 || y - h / 2 < 0 || x + w / 2 > sourceimgIn.cols || y + h / 2 > sourceimgIn.rows)
    throw std::runtime_error("roicrop out of bounds");

  cv::Rect roi = cv::Rect(x - w / 2, y - h / 2, w, h);
  cv::Mat crop = sourceimgIn(roi);
  return crop.clone();
}

inline cv::Mat roicropmid(const cv::Mat& sourceimgIn, int w, int h)
{
  return roicrop(sourceimgIn, sourceimgIn.cols / 2, sourceimgIn.rows / 2, w, h);
}

inline double magnitude(const cv::Point2f& pt)
{
  return sqrt(sqr(pt.x) + sqr(pt.y));
}

inline double angle(const cv::Point2f& pt)
{
  return toDegrees(atan2(pt.y, pt.x));
}

inline std::pair<double, double> minMaxMat(const cv::Mat& sourceimg)
{
  double minR, maxR;
  minMaxLoc(sourceimg, &minR, &maxR, nullptr, nullptr);
  return std::make_pair(minR, maxR);
}

inline std::string to_string(const cv::Point2d& point)
{
  return std::string("[" + std::to_string(point.x) + "," + std::to_string(point.y) + "]");
}

inline cv::Point2f findCentroid(const cv::Mat& sourceimg)
{
  double M = 0.0;
  double My = 0.0;
  double Mx = 0.0;
  for (int r = 0; r < sourceimg.rows; r++)
  {
    for (int c = 0; c < sourceimg.cols; c++)
    {
      M += sourceimg.at<float>(r, c);
      My += (double)r * sourceimg.at<float>(r, c);
      Mx += (double)c * sourceimg.at<float>(r, c);
    }
  }

  cv::Point2f ret(Mx / M, My / M);

  if (ret.x < 0 || ret.y < 0 || ret.x > sourceimg.cols || ret.y > sourceimg.rows)
    return cv::Point2f(sourceimg.cols / 2, sourceimg.rows / 2);
  else
    return ret;
}

inline cv::Point2d median(std::vector<cv::Point2d>& vec)
{
  // function changes the vec order, watch out
  std::sort(vec.begin(), vec.end(), [](cv::Point2d a, cv::Point2d b) { return a.x < b.x; });
  return vec[vec.size() / 2];
}

inline cv::Point2f mean(const std::vector<cv::Point2f>& vec)
{
  cv::Point2f mean(0, 0);
  for (auto& x : vec)
    mean += x;
  return mean * (1. / vec.size());
}

inline std::vector<double> GetMidRow(const cv::Mat& img)
{
  std::vector<double> out(img.cols);
  for (int c = 0; c < img.cols; ++c)
    out[c] = img.at<float>(img.rows / 2, c);
  return out;
}

inline std::vector<double> GetMidCol(const cv::Mat& img)
{
  std::vector<double> out(img.rows);
  for (int r = 0; r < img.rows; ++r)
    out[r] = img.at<float>(r, img.cols / 2);
  return out;
}

inline std::vector<double> GetDiagonal(const cv::Mat& img)
{
  std::vector<double> out(img.cols);
  for (int c = 0; c < img.cols; ++c)
    out[c] = img.at<float>(c, c);
  return out;
}

inline void Shift(cv::Mat& image, const cv::Point2f& shift)
{
  cv::Mat T = (cv::Mat_<float>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
  warpAffine(image, image, T, image.size());
}

inline void Shift(cv::Mat& image, float shiftx, float shifty)
{
  Shift(image, {shiftx, shifty});
}

inline void Rotate(cv::Mat& image, float rot, float scale = 1)
{
  cv::Point2f center((float)image.cols / 2, (float)image.rows / 2);
  cv::Mat R = getRotationMatrix2D(center, rot, scale);
  warpAffine(image, image, R, image.size());
}

template <>
struct fmt::formatter<cv::Point2f>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Point2f& point, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "[{}, {}]", point.x, point.y);
  }
};

template <>
struct fmt::formatter<cv::Size>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Size& size, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "[{}, {}]", size.width, size.height);
  }
};

template <>
struct fmt::formatter<cv::MatSize>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::MatSize& size, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{}", size());
  }
};

template <>
struct fmt::formatter<cv::Mat>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Mat& mat, FormatContext& ctx)
  {
    if (mat.rows == 0)
      return fmt::format_to(ctx.out(), "[]");

    fmt::format_to(ctx.out(), "[");
    for (int r = 0; r < mat.rows; ++r)
    {
      for (int c = 0; c < mat.cols; ++c)
        fmt::format_to(ctx.out(), "{}, ", mat.at<float>(r, c));
      fmt::format_to(ctx.out(), "\n");
    }

    return fmt::format_to(ctx.out(), "]");
  }
};