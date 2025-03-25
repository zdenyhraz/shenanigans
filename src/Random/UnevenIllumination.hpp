#pragma once
#include "Math/Fourier.hpp"
#include "Plot/Plot.hpp"

void CorrectUnevenIlluminationCLAHE(const cv::Mat& image, int tileGridSize, int clipLimit)
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

  Plot::Plot("input", image);
  Plot::Plot("clahe output", imageCLAHE);
}

void CorrectUnevenIlluminationHomomorphic(const cv::Mat& image, double cutoff = 0.001)
{
  cv::Mat imageLAB;
  cv::cvtColor(image, imageLAB, cv::COLOR_BGR2Lab);
  std::vector<cv::Mat> LABplanes(3);
  cv::split(imageLAB, LABplanes);

  cv::Mat lightness = LABplanes[0].clone();
  lightness.convertTo(lightness, GetMatType<double>());

  cv::log(lightness, lightness);
  cv::Mat fft = FFT(lightness);
  cv::Mat filter = 1. - Butterworth<double>(lightness.size(), cutoff, 1);
  IFFTShift(filter);
  cv::multiply(fft, DuplicateChannelsCopy(filter), fft);
  lightness = IFFT(fft);
  cv::exp(lightness, lightness);

  cv::normalize(lightness, lightness, 0, 255, cv::NORM_MINMAX);
  lightness.convertTo(lightness, LABplanes[0].type());
  LABplanes[0] = lightness;

  cv::merge(LABplanes, imageLAB);
  cv::Mat result;
  cv::cvtColor(imageLAB, result, cv::COLOR_Lab2BGR);

  Plot::Plot("homo output", result);
}
