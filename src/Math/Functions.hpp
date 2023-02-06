#pragma once

inline f64 Gaussian(f64 x, f64 amp, f64 mid, f64 sigma)
{
  return amp * std::exp(-0.5 * std::pow((x - mid) / sigma, 2));
}

template <typename T>
inline cv::Mat Gaussian(i32 size, f64 stddev)
{
  cv::Mat mat(size, size, GetMatType<T>());
  for (i32 row = 0; row < size; ++row)
  {
    auto matp = mat.ptr<T>(row);
    for (i32 col = 0; col < size; ++col)
    {
      f64 r = std::sqrt(Sqr(row - size / 2) + Sqr(col - size / 2));
      matp[col] = Gaussian(r, 1, 0, stddev);
    }
  }
  return mat;
}

template <typename T>
inline cv::Mat Kirkl(i32 rows, i32 cols, f64 radius)
{
  cv::Mat mat = cv::Mat(rows, cols, GetMatType<T>());
  const i32 radsq = Sqr(radius);
  const i32 rowsh = rows / 2;
  const i32 colsh = cols / 2;
  for (i32 r = 0; r < rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (i32 c = 0; c < cols; ++c)
      matp[c] = (Sqr(r - rowsh) + Sqr(c - colsh)) <= radsq;
  }
  return mat;
}

template <typename T>
inline cv::Mat Kirkl(i32 size)
{
  return Kirkl<T>(size, size, 0.5 * size);
}
