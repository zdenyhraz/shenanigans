#pragma once

inline cv::Mat RoiCropRef(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  if (x < 0 or y < 0 or x - w / 2 < 0 or y - h / 2 < 0 or x + w / 2 > mat.cols or y + h / 2 > mat.rows)
    [[unlikely]] throw std::runtime_error("RoiCrop out of bounds");

  return mat(cv::Rect(x - w / 2, y - h / 2, w, h));
}

inline cv::Mat RoiCrop(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  PROFILE_FUNCTION;
  return RoiCropRef(mat, x, y, w, h).clone();
}

inline cv::Mat RoiCropMid(const cv::Mat& mat, i32 w, i32 h)
{
  PROFILE_FUNCTION;
  return RoiCrop(mat, mat.cols / 2, mat.rows / 2, w, h);
}

inline cv::Mat RoiCropClipRef(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  if (x < 0 or y < 0 or x >= mat.cols or y >= mat.rows)
    [[unlikely]] throw std::runtime_error("RoiCropClip out of bounds");

  // top left point inclusive, bot right point exclusive
  const i32 ulx = x - w / 2;
  const i32 uly = y - h / 2;
  const i32 ulxc = std::clamp(ulx, 0, mat.cols - 1);
  const i32 ulyc = std::clamp(uly, 0, mat.rows - 1);
  const i32 brxc = std::clamp(ulx + w, 0, mat.cols);
  const i32 bryc = std::clamp(uly + h, 0, mat.rows);

  return mat(cv::Rect(cv::Point(ulxc, ulyc), cv::Point(brxc, bryc)));
}

inline cv::Mat RoiCropClip(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  PROFILE_FUNCTION;
  return RoiCropClipRef(mat, x, y, w, h).clone();
}

template <typename T>
inline cv::Mat RoiCropRep(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  PROFILE_FUNCTION;
  cv::Mat roi(h, w, GetMatType<T>());
  for (i32 r = 0; r < roi.rows; ++r)
    for (i32 c = 0; c < roi.cols; ++c)
      roi.at<T>(r, c) = mat.at<T>(std::clamp(y - h / 2 + r, 0, mat.rows - 1), std::clamp(x - w / 2 + c, 0, mat.cols - 1));
  return roi;
}

template <typename T>
inline cv::Mat Kirkl(i32 rows, i32 cols, f64 radius)
{
  PROFILE_FUNCTION;
  cv::Mat mat = cv::Mat(rows, cols, GetMatType<T>());
  const i32 radsq = sqr(radius);
  const i32 rowsh = rows / 2;
  const i32 colsh = cols / 2;
  for (i32 r = 0; r < rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (i32 c = 0; c < cols; ++c)
      matp[c] = (sqr(r - rowsh) + sqr(c - colsh)) <= radsq;
  }
  return mat;
}

template <typename T>
inline cv::Mat Kirkl(i32 size)
{
  return Kirkl<T>(size, size, 0.5 * size);
}

template <typename T>
inline cv::Mat KirklCrop(const cv::Mat& mat, i32 x, i32 y, i32 diameter)
{
  PROFILE_FUNCTION;
  return RoiCrop(mat, x, y, diameter, diameter).mul(Kirkl<T>(diameter));
}
