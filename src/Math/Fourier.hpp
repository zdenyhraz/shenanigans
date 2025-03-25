#pragma once
#include "Math/Functions.hpp"

inline cv::Mat FFT(cv::Mat&& img)
{
  PROFILE_FUNCTION;
  cv::dft(img, img, cv::DFT_COMPLEX_OUTPUT);
  return img;
}

inline cv::Mat IFFT(cv::Mat&& FFT)
{
  PROFILE_FUNCTION;
  cv::dft(FFT, FFT, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
  return FFT;
}

inline cv::Mat GPUFFT(cv::Mat&& img)
{
  PROFILE_FUNCTION;
  /*
  cv::cuda::GpuMat imgGpu;
  imgGpu.upload(img);
  cv::cuda::dft(imgGpu, imgGpu, imgGpu.size());
  imgGpu.download(img);
  */
  return img;
}

inline cv::Mat GPUIFFT(cv::Mat&& FFT)
{
  PROFILE_FUNCTION;
  /*
  cv::cuda::GpuMat fftGpu;
  fftGpu.upload(FFT);
  cv::cuda::dft(fftGpu, fftGpu, fftGpu.size(), cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
  fftGpu.download(FFT);
  */
  return FFT;
}

inline cv::Mat FFT(cv::Mat& img)
{
  return FFT(img.clone());
}

inline cv::Mat IFFT(cv::Mat& FFT)
{
  return IFFT(FFT.clone());
}

inline cv::Mat GPUFFT(cv::Mat& img)
{
  return GPUFFT(img.clone());
}

inline cv::Mat GPUIFFT(cv::Mat& FFT)
{
  return GPUIFFT(FFT.clone());
}

inline void FFTShift(cv::Mat& mat)
{
  PROFILE_FUNCTION;
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

inline cv::Mat FFTShift(cv::Mat&& mat)
{
  FFTShift(mat);
  return mat;
}

inline void IFFTShift(cv::Mat& mat)
{
  PROFILE_FUNCTION;
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

inline cv::Mat DuplicateChannelsCopy(const cv::Mat& img)
{
  cv::Mat out;
  cv::Mat planes[] = {img, img};
  merge(planes, 2, out);
  return out;
}

inline cv::Mat DuplicateChannelsZero(const cv::Mat& img)
{
  cv::Mat out;
  cv::Mat planes[] = {img, cv::Mat::zeros(img.size(), CV_32F)};
  merge(planes, 2, out);
  return out;
}

inline cv::Mat LogMagnitude(const cv::Mat& img, int logs = 1)
{
  PROFILE_FUNCTION;
  cv::Mat mag;
  if (img.channels() > 1)
  {
    cv::Mat planes[2];
    split(img, planes);
    cv::magnitude(planes[0], planes[1], mag);
  }
  else
  {
    mag = img.clone();
  }
  for (int logit = 0; logit < logs; ++logit)
  {
    mag += cv::Scalar::all(1);
    log(mag, mag);
    cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
  }
  return mag;
}

inline cv::Mat Magnitude(const cv::Mat& img)
{
  PROFILE_FUNCTION;
  if (img.channels() != 2)
    throw std::runtime_error("Need two channels for magnitude info");

  cv::Mat mgn;
  cv::Mat planes[2];
  split(img, planes);
  cv::magnitude(planes[0], planes[1], mgn);
  return mgn;
}

inline cv::Mat Phase(const cv::Mat& img)
{
  PROFILE_FUNCTION;
  if (img.channels() != 2)
    throw std::runtime_error("Need two channels for Phase info");

  cv::Mat phs;
  cv::Mat planes[2];
  split(img, planes);
  cv::phase(planes[0], planes[1], phs);
  return phs;
}

inline cv::Mat FFTLogMagnitude(const cv::Mat& img, int logs = 1)
{
  PROFILE_FUNCTION;
  cv::Mat out = FFT(img.clone());
  FFTShift(out);
  return LogMagnitude(out, logs);
}

inline cv::Mat IFFTLogMagnitude(const cv::Mat& img, int logs = 1)
{
  PROFILE_FUNCTION;
  cv::Mat out = IFFT(DuplicateChannelsZero(img));
  FFTShift(out);
  return LogMagnitude(out, logs);
}

template <typename T>
inline cv::Mat Hanning(cv::Size size)
{
  cv::Mat mat;
  cv::createHanningWindow(mat, size, GetMatType<T>());
  return mat;
}
