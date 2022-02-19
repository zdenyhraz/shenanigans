#pragma once

inline cv::Mat LoadUnitFloatImage(const std::string& path)
{
  cv::Mat mat = cv::imread(path, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
  mat.convertTo(mat, CV_32F);
  normalize(mat, mat, 0, 1, cv::NORM_MINMAX);
  return mat;
}

inline f64 magnitude(const cv::Point2f& pt)
{
  return sqrt(sqr(pt.x) + sqr(pt.y));
}

inline f64 angle(const cv::Point2f& pt)
{
  return toDegrees(atan2(pt.y, pt.x));
}

inline std::pair<f64, f64> minMaxMat(const cv::Mat& sourceimg)
{
  f64 minR, maxR;
  minMaxLoc(sourceimg, &minR, &maxR, nullptr, nullptr);
  return std::make_pair(minR, maxR);
}

inline std::string to_string(const cv::Point2d& point)
{
  return std::string("[" + std::to_string(point.x) + "," + std::to_string(point.y) + "]");
}

inline cv::Point2f findCentroid(const cv::Mat& sourceimg)
{
  f64 M = 0.0;
  f64 My = 0.0;
  f64 Mx = 0.0;
  for (i32 r = 0; r < sourceimg.rows; r++)
  {
    for (i32 c = 0; c < sourceimg.cols; c++)
    {
      M += sourceimg.at<f32>(r, c);
      My += (f64)r * sourceimg.at<f32>(r, c);
      Mx += (f64)c * sourceimg.at<f32>(r, c);
    }
  }

  cv::Point2f ret(Mx / M, My / M);

  if (ret.x < 0 or ret.y < 0 or ret.x > sourceimg.cols or ret.y > sourceimg.rows)
    [[unlikely]] return cv::Point2f(sourceimg.cols / 2, sourceimg.rows / 2);
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

inline std::vector<f64> GetMidRow(const cv::Mat& img)
{
  std::vector<f64> out(img.cols);
  for (i32 c = 0; c < img.cols; ++c)
    out[c] = img.at<f32>(img.rows / 2, c);
  return out;
}

inline std::vector<f64> GetMidCol(const cv::Mat& img)
{
  std::vector<f64> out(img.rows);
  for (i32 r = 0; r < img.rows; ++r)
    out[r] = img.at<f32>(r, img.cols / 2);
  return out;
}

inline std::vector<f64> GetDiagonal(const cv::Mat& img)
{
  std::vector<f64> out(img.cols);
  for (i32 c = 0; c < img.cols; ++c)
    out[c] = img.at<f32>(c, c);
  return out;
}

inline void Shift(cv::Mat& image, const cv::Point2f& shift)
{
  cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
  warpAffine(image, image, T, image.size());
}

inline void Shift(cv::Mat& image, f32 shiftx, f32 shifty)
{
  Shift(image, {shiftx, shifty});
}

inline void Rotate(cv::Mat& image, f32 rot, f32 scale = 1)
{
  cv::Point2f center((f32)image.cols / 2, (f32)image.rows / 2);
  cv::Mat R = getRotationMatrix2D(center, rot, scale);
  warpAffine(image, image, R, image.size());
}

template <typename T>
inline bool Equal(const cv::Mat& mat1, const cv::Mat& mat2, f64 tolerance = 0.)
{
  if (mat1.size() != mat2.size())
    return false;
  if (mat1.channels() != mat2.channels())
    return false;
  if (mat1.depth() != mat2.depth())
    return false;
  if (mat1.type() != mat2.type())
    return false;

  for (i32 r = 0; r < mat1.rows; ++r)
    for (i32 c = 0; c < mat1.cols; ++c)
      if (static_cast<f64>(std::abs(mat1.at<T>(r, c) - mat2.at<T>(r, c))) > tolerance)
        return false;

  return true;
}