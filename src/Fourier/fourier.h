#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Core/constants.h"

namespace Fourier
{
inline cv::Mat fft(cv::Mat&& img, bool packed = false)
{
  if (img.type() != CV_32F)
    img.convertTo(img, CV_32F);

  if (packed)
    dft(img, img);
  else
    dft(img, img, cv::DFT_COMPLEX_OUTPUT);

  return img;
}

inline cv::Mat ifft(cv::Mat&& fft, bool packed = false)
{
  dft(fft, fft, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
  return fft;
}

inline cv::Mat cufft(cv::Mat&& img, bool packed = false)
{
  if (img.type() != CV_32F)
    img.convertTo(img, CV_32F);

  // cuda::GpuMat imgGpu;

  if (packed)
  {
    LOG_ERROR("cufft packed input channels: {}", img.channels());
    // imgGpu.upload(img);
    // cuda::dft(imgGpu, imgGpu, imgGpu.size());
    // imgGpu.download(img);
    LOG_ERROR("cufft packed output channels: {}", img.channels());
    return img;
  }
  else
  {
    cv::Mat planes[] = {img, cv::Mat::zeros(img.size(), CV_32F)};
    merge(planes, 2, img);
    // imgGpu.upload(img);
    // cuda::dft(imgGpu, imgGpu, imgGpu.size());
    // imgGpu.download(img);
    return img;
  }
}

inline cv::Mat icufft(cv::Mat&& fft, bool packed = false)
{
  // cuda::GpuMat fftGpu;
  // fftGpu.upload(fft);

  if (packed)
  {
    // cuda::dft(fftGpu, fftGpu, fftGpu.size(), cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
    // fftGpu.download(fft);
    return fft;
  }
  else
  {
    // cuda::dft(fftGpu, fftGpu, fftGpu.size(), cv::DFT_INVERSE | cv::DFT_SCALE);
    // fftGpu.download(fft);
    cv::Mat out;
    extractChannel(fft, out, 0);
    return out;
  }
}

inline cv::Mat fft(const cv::Mat& img, bool packed = false)
{
  return fft(img.clone(), packed);
}

inline cv::Mat ifft(const cv::Mat& fft, bool packed = false)
{
  return ifft(fft.clone(), packed);
}

inline cv::Mat cufft(const cv::Mat& img, bool packed = false)
{
  return cufft(img.clone(), packed);
}

inline cv::Mat icufft(const cv::Mat& fft, bool packed = false)
{
  return icufft(fft.clone(), packed);
}

inline void fftshift(cv::Mat& mat)
{
  int cx = mat.cols / 2;
  int cy = mat.rows / 2;
  cv::Mat q0(mat, cv::Rect(0, 0, cx, cy));
  cv::Mat q1(mat, cv::Rect(cx, 0, cx, cy));
  cv::Mat q2(mat, cv::Rect(0, cy, cx, cy));
  cv::Mat q3(mat, cv::Rect(cx, cy, cx, cy));

  cv::Mat tmp;
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);
  q1.copyTo(tmp);
  q2.copyTo(q1);
  tmp.copyTo(q2);
}

inline void ifftshift(cv::Mat& mat)
{
  int cx = mat.cols / 2;
  int cy = mat.rows / 2;
  cv::Mat q0(mat, cv::Rect(0, 0, cx, cy));
  cv::Mat q1(mat, cv::Rect(cx, 0, cx, cy));
  cv::Mat q2(mat, cv::Rect(0, cy, cx, cy));
  cv::Mat q3(mat, cv::Rect(cx, cy, cx, cy));

  cv::Mat tmp;
  q3.copyTo(tmp);
  q0.copyTo(q3);
  tmp.copyTo(q0);
  q2.copyTo(tmp);
  q1.copyTo(q2);
  tmp.copyTo(q1);
}

inline cv::Mat dupchansc(const cv::Mat& img)
{
  cv::Mat out;
  cv::Mat planes[] = {img, img};
  merge(planes, 2, out);
  return out;
}

inline cv::Mat dupchansz(const cv::Mat& img)
{
  cv::Mat out;
  cv::Mat planes[] = {img, cv::Mat::zeros(img.size(), CV_32F)};
  merge(planes, 2, out);
  return out;
}

inline cv::Mat logmagn(const cv::Mat& img, int logs = 1)
{
  cv::Mat mag;
  if (img.channels() > 1)
  {
    cv::Mat planes[2];
    split(img, planes);
    magnitude(planes[0], planes[1], mag);
  }
  else
  {
    mag = img.clone();
  }
  for (int logit = 0; logit < logs; ++logit)
  {
    mag += cv::Scalar::all(1);
    log(mag, mag);
    normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
  }
  return mag;
}

inline cv::Mat magn(const cv::Mat& img)
{
  return logmagn(img, 0);
}

inline cv::Mat phase(const cv::Mat& img)
{
  if (img.channels() != 2)
    throw std::runtime_error("Need two channels for phase info");

  cv::Mat phs;
  cv::Mat planes[2];
  split(img, planes);
  phase(planes[0], planes[1], phs);

  return phs;
}

inline cv::Mat fftlogmagn(const cv::Mat& img, int logs = 1)
{
  cv::Mat out = fft(img);
  fftshift(out);
  return logmagn(out, logs);
}

inline cv::Mat ifftlogmagn(const cv::Mat& img, int logs = 1)
{
  cv::Mat out = ifft(dupchansz(img), false);
  fftshift(out);
  return logmagn(out, logs);
}
}

// ------------------------------------------------ legacy code ------------------------------------------------

inline cv::Mat quadrantswap(const cv::Mat& sourceimgDFT)
{
  cv::Mat centeredDFT = sourceimgDFT.clone();
  int cx = centeredDFT.cols / 2;
  int cy = centeredDFT.rows / 2;
  cv::Mat q1(centeredDFT, cv::Rect(0, 0, cx, cy));
  cv::Mat q2(centeredDFT, cv::Rect(cx, 0, cx, cy));
  cv::Mat q3(centeredDFT, cv::Rect(0, cy, cx, cy));
  cv::Mat q4(centeredDFT, cv::Rect(cx, cy, cx, cy));
  cv::Mat temp;

  q1.copyTo(temp);
  q4.copyTo(q1);
  temp.copyTo(q4);

  q2.copyTo(temp);
  q3.copyTo(q2);
  temp.copyTo(q3);
  return centeredDFT;
}

inline cv::Mat fourier(cv::Mat&& img)
{
  img.convertTo(img, CV_32F);
  cv::Mat sourceimgcomplex[2] = {cv::Mat_<float>(img), cv::Mat::zeros(img.size(), CV_32F)};
  cv::Mat sourceimgcomplexmerged;
  merge(sourceimgcomplex, 2, sourceimgcomplexmerged);
  dft(sourceimgcomplexmerged, sourceimgcomplexmerged);
  return sourceimgcomplexmerged;
}

inline cv::Mat ifourier(cv::Mat&& img)
{
  cv::Mat out;
  dft(img, out, cv::DFT_INVERSE + cv::DFT_SCALE + cv::DFT_REAL_OUTPUT);
  return out;
}

inline cv::Mat fourier(const cv::Mat& sourceimgIn)
{
  cv::Mat img = sourceimgIn.clone();
  return fourier(std::move(img));
}

inline cv::Mat ifourier(const cv::Mat& sourceimgIn)
{
  cv::Mat img = sourceimgIn.clone();
  return ifourier(std::move(img));
}

inline cv::Mat fourierinv(const cv::Mat& realIn, const cv::Mat& imagIn)
{
  cv::Mat real = realIn.clone();
  cv::Mat imag = imagIn.clone();
  cv::Mat invDFT;
  cv::Mat DFTcomplex[2] = {real, imag};
  cv::Mat DFTcomplexmerged;
  merge(DFTcomplex, 2, DFTcomplexmerged);
  dft(DFTcomplexmerged, invDFT, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
  normalize(invDFT, invDFT, 0, 65535, cv::NORM_MINMAX);
  invDFT.convertTo(invDFT, CV_16UC1);
  return invDFT;
}

inline cv::Mat edgemask(int rows, int cols)
{
  cv::Mat edgemask;
  createHanningWindow(edgemask, cv::Size(cols, rows), CV_32F);
  return edgemask;
}

inline cv::Mat gaussian(int rows, int cols, double stdevYmult, double stdevXmult)
{
  cv::Mat gaussian = cv::Mat::zeros(rows, cols, CV_32F);
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
      gaussian.at<float>(r, c) = std::exp(-(std::pow(c - cols / 2, 2) / 2 / std::pow((double)cols / stdevXmult, 2) + std::pow(r - rows / 2, 2) / 2 / std::pow((double)rows / stdevYmult, 2)));

  normalize(gaussian, gaussian, 0, 1, cv::NORM_MINMAX);
  return gaussian;
}

inline cv::Mat laplacian(int rows, int cols, double stdevYmult, double stdevXmult)
{
  cv::Mat laplacian = cv::Mat::ones(rows, cols, CV_32F);
  laplacian = 1 - gaussian(rows, cols, stdevYmult, stdevXmult);
  normalize(laplacian, laplacian, 0, 1, cv::NORM_MINMAX);
  return laplacian;
}

inline cv::Mat bandpassian(int rows, int cols, double stdevLmult, double stdevHmult)
{
  cv::Mat bandpassian = gaussian(rows, cols, stdevLmult, stdevLmult).mul(laplacian(rows, cols, 1. / stdevHmult, 1. / stdevHmult));
  normalize(bandpassian, bandpassian, 0, 1, cv::NORM_MINMAX);
  return bandpassian;
}

inline cv::Mat sinian(int rows, int cols, double frequencyX, double frequencyY)
{
  cv::Mat sinian = cv::Mat::zeros(rows, cols, CV_32F);
  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < cols; x++)
    {
      sinian.at<float>(y, x) = std::sin(2 * Constants::Pi * (y + x) * frequencyX); // sin or cos just cahnges the phase spectum
    }
  }
  normalize(sinian, sinian, 0, 1, cv::NORM_MINMAX);
  sinian = sinian.mul(edgemask(rows, cols));
  return sinian;
}

inline cv::Mat bandpass(const cv::Mat& sourceimgDFTIn, const cv::Mat& bandpassMat)
{
  cv::Mat sourceimgDFT = sourceimgDFTIn.clone();
  cv::Mat filterGS = quadrantswap(bandpassMat);
  cv::Mat filter;
  cv::Mat filterPlanes[2] = {filterGS, filterGS};
  merge(filterPlanes, 2, filter);
  return sourceimgDFT.mul(filter);
}

void showfourier(const cv::Mat& DFTimgIn, bool logar = true, bool expon = false, std::string magnwindowname = "FFTmagn", std::string phasewindowname = "FFTphase");

cv::Mat convolute(cv::Mat img, cv::Mat PSFimg);

cv::Mat deconvolute(cv::Mat img, cv::Mat PSFimg);

cv::Mat deconvoluteWiener(const cv::Mat& img, const cv::Mat& PSFimg);

cv::Mat frequencyFilter(const cv::Mat& img, const cv::Mat& mask);
