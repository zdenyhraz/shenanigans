template <typename T>
inline T GetMedian(const cv::Mat& mat, std::vector<T>& values)
{
  for (i32 r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (i32 c = 0; c < mat.cols; ++c)
      values[r * mat.cols + c] = matp[c];
  }

  return Median(values);
}

template <typename T>
inline cv::Mat MedianBlur(const cv::Mat& mat, i32 kwidth, i32 kheight)
{
  if (kwidth % 2 == 0 or kheight % 2 == 0)
    [[unlikely]] throw std::invalid_argument("Invalid median blur window size");

  cv::Mat med = cv::Mat(mat.size(), GetMatType<T>());
  std::vector<T> values(kwidth * kheight); // buffer for median calculation to prevent re-allocations
  const i32 kwidth_ = std::min(kwidth, mat.cols % 2 ? mat.cols : mat.cols - 1);
  const i32 kheight_ = std::min(kheight, mat.rows % 2 ? mat.rows : mat.rows - 1);
  for (i32 r = 0; r < mat.rows; ++r)
  {
    auto medp = med.ptr<T>(r);
    for (i32 c = 0; c < mat.cols; ++c)
      medp[c] = GetMedian<T>(roicroprep<T>(mat, c, r, kwidth_, kheight_), values);
  }
  return med;
}