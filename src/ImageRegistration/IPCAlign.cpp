#include "IPCAlign.hpp"
#include "IPC.hpp"

cv::Mat IPCAlign::Align(const IPC& ipc, const cv::Mat& image1, const cv::Mat& image2)
{
  return Align(ipc, image1.clone(), image2.clone());
}

cv::Mat IPCAlign::Align(const IPC& ipc, cv::Mat&& image1, cv::Mat&& image2)
{
  PROFILE_FUNCTION;
  static constexpr bool debugMode = true;
  if constexpr (debugMode)
    IPCDebug::DebugInputImages(ipc, image1, image2);

  cv::Mat img1W = image1.clone();
  cv::Mat img2W = image2.clone();
  ipc.ApplyWindow(img1W);
  ipc.ApplyWindow(img2W);
  cv::Mat img1FT = Fourier::fft(img1W);
  cv::Mat img2FT = Fourier::fft(img2W);
  Fourier::fftshift(img1FT);
  Fourier::fftshift(img2FT);
  cv::Mat img1FTm = cv::Mat(img1FT.size(), GetMatType<IPC::Float>());
  cv::Mat img2FTm = cv::Mat(img2FT.size(), GetMatType<IPC::Float>());
  for (i32 row = 0; row < img1FT.rows; ++row)
  {
    auto img1FTp = img1FT.ptr<cv::Vec<IPC::Float, 2>>(row);
    auto img2FTp = img2FT.ptr<cv::Vec<IPC::Float, 2>>(row);
    auto img1FTmp = img1FTm.ptr<IPC::Float>(row);
    auto img2FTmp = img2FTm.ptr<IPC::Float>(row);
    for (i32 col = 0; col < img1FT.cols; ++col)
    {
      const auto& re1 = img1FTp[col][0];
      const auto& im1 = img1FTp[col][1];
      const auto& re2 = img2FTp[col][0];
      const auto& im2 = img2FTp[col][1];
      img1FTmp[col] = std::log(std::sqrt(re1 * re1 + im1 * im1));
      img2FTmp[col] = std::log(std::sqrt(re2 * re2 + im2 * im2));
    }
  }
  cv::Point2d center(0.5 * image1.cols, 0.5 * image1.rows);
  f64 maxRadius = std::min(center.y, center.x);
  cv::warpPolar(img1FTm, img1FTm, img1FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar
  cv::warpPolar(img2FTm, img2FTm, img2FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar
  const auto gamma1 = 1.0;
  const auto gamma2 = 1.0;
  cv::Mat showImage;

  if constexpr (debugMode)
  {
    showImage = ColorComposition(image1, image1 * 0, gamma1, gamma2);
    cv::hconcat(showImage, ColorComposition(image2 * 0, image2, gamma1, gamma2), showImage);
    cv::hconcat(showImage, ColorComposition(image1, image2, gamma1, gamma2), showImage);
  }

  // rotation and scale
  auto shiftR = ipc.Calculate(img1FTm, img2FTm);
  f64 rotation = -shiftR.y / image1.rows * 360;
  f64 scale = std::exp(shiftR.x * std::log(maxRadius) / image1.cols);
  Rotate(image2, -rotation, scale);
  if constexpr (debugMode)
    cv::hconcat(showImage, ColorComposition(image1, image2, gamma1, gamma2), showImage);

  // translation
  auto shiftT = ipc.Calculate(image1, image2);
  Shift(image2, -shiftT);
  if constexpr (debugMode)
  {
    cv::hconcat(showImage, ColorComposition(image1, image2, gamma1, gamma2), showImage);
    Showimg(showImage, "Align process color composition");
    LOG_DEBUG("Evaluated shift: {}", shiftT);
    LOG_DEBUG("Evaluated rotation/scale: {:.3f}/{:.3f}", rotation, 1. / scale);
  }
  return image2;
}

cv::Mat IPCAlign::ColorComposition(const cv::Mat& img1, const cv::Mat& img2, f64 gamma1, f64 gamma2)
{
  PROFILE_FUNCTION;
  const cv::Vec<IPC::Float, 3> img1clr = {1, 0.5, 0};
  const cv::Vec<IPC::Float, 3> img2clr = {0, 0.5, 1};

  cv::Mat img1c = cv::Mat(img1.size(), GetMatType<IPC::Float>(3));
  cv::Mat img2c = cv::Mat(img2.size(), GetMatType<IPC::Float>(3));

  for (i32 row = 0; row < img1.rows; ++row)
  {
    auto img1p = img1.ptr<IPC::Float>(row);
    auto img2p = img2.ptr<IPC::Float>(row);
    auto img1cp = img1c.ptr<cv::Vec<IPC::Float, 3>>(row);
    auto img2cp = img2c.ptr<cv::Vec<IPC::Float, 3>>(row);

    for (i32 col = 0; col < img1.cols; ++col)
    {
      img1cp[col][0] = std::pow(img1clr[2] * img1p[col], 1. / gamma1);
      img1cp[col][1] = std::pow(img1clr[1] * img1p[col], 1. / gamma1);
      img1cp[col][2] = std::pow(img1clr[0] * img1p[col], 1. / gamma1);

      img2cp[col][0] = std::pow(img2clr[2] * img2p[col], 1. / gamma2);
      img2cp[col][1] = std::pow(img2clr[1] * img2p[col], 1. / gamma2);
      img2cp[col][2] = std::pow(img2clr[0] * img2p[col], 1. / gamma2);
    }
  }

  cv::normalize(img1c, img1c, 0, 1, cv::NORM_MINMAX);
  cv::normalize(img2c, img2c, 0, 1, cv::NORM_MINMAX);
  return (img1c + img2c) / 2;
}
