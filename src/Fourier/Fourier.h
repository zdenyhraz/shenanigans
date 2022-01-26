#pragma once
#include "Utils/FunctionsBaseSTL.h"
#include "UtilsCV/FunctionsBaseCV.h"
#include "Utils/Constants.h"

namespace Fourier
{
inline cv::Mat fft(cv::Mat&& img, bool packed = false)
{
  if (img.type() != CV_32F)
    [[unlikely]] img.convertTo(img, CV_32F);

  if (packed)
    [[unlikely]] cv::dft(img, img);
  else
    cv::dft(img, img, cv::DFT_COMPLEX_OUTPUT);

  return img;
}

inline cv::Mat ifft(cv::Mat&& fft)
{
  cv::dft(fft, fft, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
  return fft;
}

inline cv::Mat gpufft(cv::Mat&& img)
{
  if (img.type() != CV_32F)
    img.convertTo(img, CV_32F);

  /*
  cv::cuda::GpuMat imgGpu;
  imgGpu.upload(img);
  cv::cuda::dft(imgGpu, imgGpu, imgGpu.size());
  imgGpu.download(img);
  */
  return img;
}

inline cv::Mat gpuifft(cv::Mat&& fft)
{
  /*
  cv::cuda::GpuMat fftGpu;
  fftGpu.upload(fft);
  cv::cuda::dft(fftGpu, fftGpu, fftGpu.size(), cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
  fftGpu.download(fft);
  */
  return fft;
}

inline cv::Mat fft(cv::Mat& img, bool packed = false)
{
  return fft(img.clone(), packed);
}

inline cv::Mat ifft(cv::Mat& fft)
{
  return ifft(fft.clone());
}

inline cv::Mat gpufft(cv::Mat& img)
{
  return gpufft(img.clone());
}

inline cv::Mat gpuifft(cv::Mat& fft)
{
  return gpuifft(fft.clone());
}

inline void fftshift(cv::Mat& mat)
{
  i32 cx = mat.cols / 2;
  i32 cy = mat.rows / 2;
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

inline cv::Mat fftshift(cv::Mat&& mat)
{
  fftshift(mat);
  return mat;
}

inline void ifftshift(cv::Mat& mat)
{
  i32 cx = mat.cols / 2;
  i32 cy = mat.rows / 2;
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

inline cv::Mat logmagn(const cv::Mat& img, i32 logs = 1)
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
  for (i32 logit = 0; logit < logs; ++logit)
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

inline cv::Mat fftlogmagn(const cv::Mat& img, i32 logs = 1)
{
  cv::Mat out = fft(img.clone());
  fftshift(out);
  return logmagn(out, logs);
}

inline cv::Mat ifftlogmagn(const cv::Mat& img, i32 logs = 1)
{
  cv::Mat out = ifft(dupchansz(img));
  fftshift(out);
  return logmagn(out, logs);
}
}

// ------------------------------------------------ legacy code ------------------------------------------------

inline cv::Mat quadrantswap(const cv::Mat& sourceimgDFT)
{
  cv::Mat centeredDFT = sourceimgDFT.clone();
  i32 cx = centeredDFT.cols / 2;
  i32 cy = centeredDFT.rows / 2;
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
  cv::Mat sourceimgcomplex[2] = {cv::Mat_<f32>(img), cv::Mat::zeros(img.size(), CV_32F)};
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

inline cv::Mat edgemask(i32 rows, i32 cols)
{
  cv::Mat edgemask;
  createHanningWindow(edgemask, cv::Size(cols, rows), CV_32F);
  return edgemask;
}

inline cv::Mat gaussian(i32 rows, i32 cols, f64 stdevYmult, f64 stdevXmult)
{
  cv::Mat gaussian = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
    for (i32 c = 0; c < cols; c++)
      gaussian.at<f32>(r, c) = std::exp(-(std::pow(c - cols / 2, 2) / 2 / std::pow((f64)cols / stdevXmult, 2) + std::pow(r - rows / 2, 2) / 2 / std::pow((f64)rows / stdevYmult, 2)));

  normalize(gaussian, gaussian, 0, 1, cv::NORM_MINMAX);
  return gaussian;
}

inline cv::Mat laplacian(i32 rows, i32 cols, f64 stdevYmult, f64 stdevXmult)
{
  cv::Mat laplacian = cv::Mat::ones(rows, cols, CV_32F);
  laplacian = 1 - gaussian(rows, cols, stdevYmult, stdevXmult);
  normalize(laplacian, laplacian, 0, 1, cv::NORM_MINMAX);
  return laplacian;
}

inline cv::Mat bandpassian(i32 rows, i32 cols, f64 stdevLmult, f64 stdevHmult)
{
  cv::Mat bandpassian = gaussian(rows, cols, stdevLmult, stdevLmult).mul(laplacian(rows, cols, 1. / stdevHmult, 1. / stdevHmult));
  normalize(bandpassian, bandpassian, 0, 1, cv::NORM_MINMAX);
  return bandpassian;
}

inline cv::Mat sinian(i32 rows, i32 cols, f64 frequencyX, f64 frequencyY)
{
  cv::Mat sinian = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 y = 0; y < rows; y++)
  {
    for (i32 x = 0; x < cols; x++)
    {
      sinian.at<f32>(y, x) = std::sin(2 * Constants::Pi * (x * frequencyX + y * frequencyY)); // sin or cos just cahnges the phase spectum
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

void showfourier(const cv::Mat& DFTimgIn, bool logar = true, bool expon = false, const std::string& magnwindowname = "FFTmagn", const std::string& phasewindowname = "FFTphase");

cv::Mat convolute(cv::Mat img, cv::Mat PSFimg);

cv::Mat deconvolute(cv::Mat img, cv::Mat PSFimg);

cv::Mat deconvoluteWiener(const cv::Mat& img, const cv::Mat& PSFimg);

cv::Mat frequencyFilter(const cv::Mat& img, const cv::Mat& mask);
