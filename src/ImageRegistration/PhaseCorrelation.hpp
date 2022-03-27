#pragma once
#include "Fourier/Fourier.hpp"

class PhaseCorrelation
{
public:
  using Float = f64;

  static cv::Point2d Calculate(const cv::Mat& image1, const cv::Mat& image2) const
  {
    PROFILE_FUNCTION;
    return Calculate(image1.clone(), image2.clone());
  }

  static cv::Point2d Calculate(cv::Mat&& image1, cv::Mat&& image2) const
  {
    PROFILE_FUNCTION;

    ConvertToUnitFloat(image1);
    ConvertToUnitFloat(image2);

    ApplyWindow(image1);
    ApplyWindow(image2);

    auto dft1 = CalculateFourierTransform(std::move(image1));
    auto dft2 = CalculateFourierTransform(std::move(image2));
    auto crosspower = CalculateCrossPowerSpectrum(std::move(dft1), std::move(dft2));
    cv::Mat L3 = CalculateL3(std::move(crosspower));

    cv::Point2d L3peak = GetPeak(L3);
    cv::Point2d L3mid(L3.cols / 2, L3.rows / 2);
    return GetPixelShift(L3peak, L3mid);
  }

private:
  static void ConvertToUnitFloat(cv::Mat& image)
  {
    PROFILE_FUNCTION;

    if (image.type() != GetMatType<Float>())
      image.convertTo(image, GetMatType<Float>());
  }

  static void ApplyWindow(cv::Mat& image)
  {
    PROFILE_FUNCTION;
    cv::Mat win;
    cv::createHanningWindow(win, image.size(), GetMatType<Float>());
    cv::multiply(image, win, image);
  }

  static cv::Mat CalculateFourierTransform(cv::Mat&& image)
  {
    PROFILE_FUNCTION;
    return Fourier::fft(std::move(image));
  }

  static cv::Mat CalculateCrossPowerSpectrum(cv::Mat&& dft1, cv::Mat&& dft2)
  {
    PROFILE_FUNCTION;
    for (i32 row = 0; row < dft1.rows; ++row)
    {
      auto dft1p = dft1.ptr<cv::Vec<Float, 2>>(row); // reuse dft1 memory
      const auto dft2p = dft2.ptr<cv::Vec<Float, 2>>(row);
      for (i32 col = 0; col < dft1.cols; ++col)
      {
        const Float re = dft1p[col][0] * dft2p[col][0] + dft1p[col][1] * dft2p[col][1];
        const Float im = dft1p[col][0] * dft2p[col][1] - dft1p[col][1] * dft2p[col][0];
        const Float mag = std::sqrt(re * re + im * im);

        dft1p[col][0] = re / mag * band;
        dft1p[col][1] = im / mag * band;
      }
    }
    return dft1;
  }

  static cv::Mat CalculateL3(cv::Mat&& crosspower)
  {
    PROFILE_FUNCTION;
    cv::Mat L3 = Fourier::ifft(std::move(crosspower));
    Fourier::fftshift(L3);
    return L3;
  }

  static cv::Point2d GetPeak(const cv::Mat& mat)
  {
    PROFILE_FUNCTION;
    cv::Point2i peak(0, 0);
    cv::minMaxLoc(mat, nullptr, nullptr, nullptr, &peak);
    return peak;
  }

  static cv::Point2d GetPixelShift(const cv::Point2d& L3peak, const cv::Point2d& L3mid) { return L3peak - L3mid; }
};
