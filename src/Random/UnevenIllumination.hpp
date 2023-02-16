#pragma once
#include "Math/Fourier.hpp"

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

  Plot::Plot("../data/UnevenIllumination/CLAHE/input.png", image);
  Plot::Plot("../data/UnevenIllumination/CLAHE/output.png", imageCLAHE);
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

  cv::log(lightness, lightness);
  cv::Mat FFT = FFT(lightness);
  cv::Mat filter = 1. - Butterworth<f64>(lightness.size(), cutoff, 1);
  IFFTShift(filter);
  cv::multiply(FFT, DuplicateChannelsCopy(filter), FFT);
  lightness = IFFT(FFT);
  cv::exp(lightness, lightness);

  cv::Mat lightness_output = lightness.clone();
  cv::normalize(lightness, lightness, 0, 255, cv::NORM_MINMAX);
  lightness.convertTo(lightness, LABplanes[0].type());
  LABplanes[0] = lightness;

  cv::merge(LABplanes, imageLAB);
  cv::Mat result;
  cv::cvtColor(imageLAB, result, cv::COLOR_Lab2BGR);

  Plot::Plot(fmt::format("../data/UnevenIllumination/Homomorphic/output_{:.3f}.png", cutoff), result);
}
