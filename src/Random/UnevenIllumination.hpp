#pragma once
#include "Fourier/Fourier.hpp"

void CorrectUnevenIlluminationCLAHE(const cv::Mat& image, i32 tileGridSize, i32 clipLimit)
{
  cv::Mat imageLAB;
  cv::cvtColor(image, imageLAB, cv::COLOR_BGR2Lab);
  std::vector<cv::Mat> LABplanes(3);
  cv::split(imageLAB, LABplanes);

  cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
  clahe->setTilesGridSize({tileGridSize, tileGridSize});
  clahe->setClipLimit(clipLimit);
  clahe->apply(LABplanes[0], LABplanes[0]);

  cv::merge(LABplanes, imageLAB);
  cv::Mat imageCLAHE;
  cv::cvtColor(imageLAB, imageCLAHE, cv::COLOR_Lab2BGR);
  cv::rectangle(imageCLAHE, cv::Rect(0, 0, tileGridSize, tileGridSize), cv::Scalar(0, 0, 255), imageCLAHE.rows * 0.005);

  Saveimg("../data/debug/UnevenIllumination/CLAHE/input.png", image);
  Saveimg("../data/debug/UnevenIllumination/CLAHE/output.png", imageCLAHE);
}

template <typename T>
cv::Mat Butterworth(cv::Size size, f64 cutoff, i32 order)
{
  cv::Mat result(size, GetMatType<T>());
  const auto D0 = cutoff * std::sqrt(Sqr(size.height / 2) + Sqr(size.width / 2));
  for (i32 r = 0; r < size.height; ++r)
  {
    for (i32 c = 0; c < size.width; ++c)
    {
      const auto D = std::sqrt(Sqr(r - size.height / 2) + Sqr(c - size.width / 2));
      result.at<T>(r, c) = 1 / (1 + std::pow(D / D0, 2 * order));
    }
  }
  return result;
}

void CorrectUnevenIlluminationHomomorphic(const cv::Mat& image, f64 cutoff = 0.001)
{
  cv::Mat imageLAB;
  cv::cvtColor(image, imageLAB, cv::COLOR_BGR2Lab);
  std::vector<cv::Mat> LABplanes(3);
  cv::split(imageLAB, LABplanes);

  cv::Mat lightness = LABplanes[0].clone();
  lightness.convertTo(lightness, GetMatType<f64>());
  cv::Mat lightness_input = lightness.clone();
  // Saveimg("../data/debug/UnevenIllumination/Homomorphic/lightness_input.png", lightness_input);

  cv::log(lightness, lightness);
  cv::Mat fft = Fourier::fft(lightness);
  cv::Mat filter = 1. - Butterworth<f64>(lightness.size(), cutoff, 1);
  // Saveimg(fmt::format("../data/debug/UnevenIllumination/Homomorphic/filter_{:.3f}.png", cutoff), filter, false, {0, 0}, true);
  Fourier::ifftshift(filter);
  cv::multiply(fft, Fourier::dupchansc(filter), fft);
  lightness = Fourier::ifft(fft);
  cv::exp(lightness, lightness);

  cv::Mat lightness_output = lightness.clone();
  // Saveimg(fmt::format("../data/debug/UnevenIllumination/Homomorphic/lightness_output_{:.3f}.png", cutoff), lightness_output);
  // Saveimg(fmt::format("../data/debug/UnevenIllumination/Homomorphic/lightness_diff_{:.3f}.png", cutoff), lightness_output - lightness_input, false, {0, 0}, true);
  cv::normalize(lightness, lightness, 0, 255, cv::NORM_MINMAX);
  lightness.convertTo(lightness, LABplanes[0].type());
  LABplanes[0] = lightness;

  cv::merge(LABplanes, imageLAB);
  cv::Mat result;
  cv::cvtColor(imageLAB, result, cv::COLOR_Lab2BGR);

  Saveimg(fmt::format("../data/debug/UnevenIllumination/Homomorphic/output_{:.3f}.png", cutoff), result);
}