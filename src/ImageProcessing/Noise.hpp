template <typename T>
cv::Mat GetNoise(cv::Size size, double stddev)
{
  PROFILE_FUNCTION;
  if (stddev <= 0)
    return cv::Mat::zeros(size, GetMatType<T>());

  cv::Mat noise = cv::Mat(size, GetMatType<T>());
  cv::randn(noise, 0, stddev);
  return noise;
}

template <typename T>
void AddNoise(cv::Mat& image, double stddev)
{
  PROFILE_FUNCTION;
  if (stddev <= 0)
    return;

  image += GetNoise<T>(image.size(), stddev);
}

template <typename T>
inline void AddNoiseCustom(cv::Mat& img, double stddev)
{
  PROFILE_FUNCTION;
  if (stddev <= 0)
    return;

  std::random_device device;
  std::mt19937 generator(device());
  std::normal_distribution<T> distribution(0., stddev);

  for (int c = 0; c < img.cols; ++c)
    for (int r = 0; r < img.rows; ++r)
      img.at<T>(r, c) = std::clamp(img.at<T>(r, c) + distribution(generator), static_cast<T>(0.), static_cast<T>(1.));
}
