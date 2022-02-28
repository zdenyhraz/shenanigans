#pragma once

namespace Fourier
{
inline cv::Mat fft(cv::Mat&& img)
{
  PROFILE_FUNCTION;
  cv::dft(img, img, cv::DFT_COMPLEX_OUTPUT);
  return img;
}

inline cv::Mat ifft(cv::Mat&& fft)
{
  PROFILE_FUNCTION;
  cv::dft(fft, fft, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
  return fft;
}

inline cv::Mat gpufft(cv::Mat&& img)
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

inline cv::Mat gpuifft(cv::Mat&& fft)
{
  PROFILE_FUNCTION;
  /*
  cv::cuda::GpuMat fftGpu;
  fftGpu.upload(fft);
  cv::cuda::dft(fftGpu, fftGpu, fftGpu.size(), cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
  fftGpu.download(fft);
  */
  return fft;
}

inline cv::Mat fft(cv::Mat& img)
{
  return fft(img.clone());
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
  PROFILE_FUNCTION;
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
  PROFILE_FUNCTION;
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
  for (i32 logit = 0; logit < logs; ++logit)
  {
    mag += cv::Scalar::all(1);
    log(mag, mag);
    cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
  }
  return mag;
}

inline cv::Mat magn(const cv::Mat& img)
{
  PROFILE_FUNCTION;
  if (img.channels() != 2)
    throw std::runtime_error("Need two channels for cv::magnitude info");

  cv::Mat mgn;
  cv::Mat planes[2];
  split(img, planes);
  cv::magnitude(planes[0], planes[1], mgn);
  return mgn;
}

inline cv::Mat phase(const cv::Mat& img)
{
  PROFILE_FUNCTION;
  if (img.channels() != 2)
    throw std::runtime_error("Need two channels for cv::phase info");

  cv::Mat phs;
  cv::Mat planes[2];
  split(img, planes);
  cv::phase(planes[0], planes[1], phs);
  return phs;
}

inline cv::Mat fftlogmagn(const cv::Mat& img, i32 logs = 1)
{
  PROFILE_FUNCTION;
  cv::Mat out = fft(img.clone());
  fftshift(out);
  return logmagn(out, logs);
}

inline cv::Mat ifftlogmagn(const cv::Mat& img, i32 logs = 1)
{
  PROFILE_FUNCTION;
  cv::Mat out = ifft(dupchansz(img));
  fftshift(out);
  return logmagn(out, logs);
}
}
