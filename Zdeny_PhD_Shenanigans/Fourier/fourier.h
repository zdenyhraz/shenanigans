#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Core/constants.h"

namespace Fourier
{
inline Mat fft(Mat&& img)
{
  if (img.type() != CV_32F)
    img.convertTo(img, CV_32F);

  Mat imgcp[] = {img, Mat::zeros(img.size(), CV_32F)};
  Mat fft;
  merge(imgcp, 2, fft);
  dft(fft, fft);
  return fft;
}

inline Mat ifft(Mat&& fft, bool conjsym = true)
{
  if (conjsym)
    dft(fft, fft, DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);
  else
    dft(fft, fft, DFT_INVERSE | DFT_SCALE);
  return fft;
}

inline Mat fft(const Mat& img)
{
  return fft(img.clone());
}

inline Mat ifft(const Mat& fft)
{
  return ifft(fft.clone());
}

inline void fftshift(Mat& mat)
{
  int cx = mat.cols / 2;
  int cy = mat.rows / 2;
  Mat q0(mat, Rect(0, 0, cx, cy));
  Mat q1(mat, Rect(cx, 0, cx, cy));
  Mat q2(mat, Rect(0, cy, cx, cy));
  Mat q3(mat, Rect(cx, cy, cx, cy));

  Mat tmp;
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);
  q1.copyTo(tmp);
  q2.copyTo(q1);
  tmp.copyTo(q2);
}

inline void ifftshift(Mat& mat)
{
  int cx = mat.cols / 2;
  int cy = mat.rows / 2;
  Mat q0(mat, Rect(0, 0, cx, cy));
  Mat q1(mat, Rect(cx, 0, cx, cy));
  Mat q2(mat, Rect(0, cy, cx, cy));
  Mat q3(mat, Rect(cx, cy, cx, cy));

  Mat tmp;
  q3.copyTo(tmp);
  q0.copyTo(q3);
  tmp.copyTo(q0);
  q2.copyTo(tmp);
  q1.copyTo(q2);
  tmp.copyTo(q1);
}

inline Mat dupchansc(const Mat& img)
{
  Mat out;
  Mat planes[] = {img, img};
  merge(planes, 2, out);
  return out;
}

inline Mat dupchansz(const Mat& img)
{
  Mat out;
  Mat planes[] = {img, Mat::zeros(img.size(), CV_32F)};
  merge(planes, 2, out);
  return out;
}

inline Mat logmagn(const Mat& img, int logs = 1)
{
  Mat mag;
  if (img.channels() > 1)
  {
    Mat planes[2];
    split(img, planes);
    magnitude(planes[0], planes[1], mag);
  }
  else
  {
    mag = img.clone();
  }
  for (int logit = 0; logit < logs; ++logit)
  {
    mag += Scalar::all(1);
    log(mag, mag);
    normalize(mag, mag, 0, 1, NORM_MINMAX);
  }
  return mag;
}

inline Mat fftlogmagn(const Mat& img, int logs = 1)
{
  Mat out = fft(img);
  fftshift(out);
  return logmagn(out, logs);
}

inline Mat ifftlogmagn(const Mat& img, int logs = 1)
{
  Mat out = ifft(dupchansz(img), false);
  fftshift(out);
  return logmagn(out, logs);
}
}

// ------------------------------------------------ legacy code ------------------------------------------------

inline Mat quadrantswap(const Mat& sourceimgDFT)
{
  Mat centeredDFT = sourceimgDFT.clone();
  int cx = centeredDFT.cols / 2;
  int cy = centeredDFT.rows / 2;
  Mat q1(centeredDFT, Rect(0, 0, cx, cy));
  Mat q2(centeredDFT, Rect(cx, 0, cx, cy));
  Mat q3(centeredDFT, Rect(0, cy, cx, cy));
  Mat q4(centeredDFT, Rect(cx, cy, cx, cy));
  Mat temp;

  q1.copyTo(temp);
  q4.copyTo(q1);
  temp.copyTo(q4);

  q2.copyTo(temp);
  q3.copyTo(q2);
  temp.copyTo(q3);
  return centeredDFT;
}

inline Mat fourier(Mat&& img)
{
  img.convertTo(img, CV_32F);
  Mat sourceimgcomplex[2] = {Mat_<float>(img), Mat::zeros(img.size(), CV_32F)};
  Mat sourceimgcomplexmerged;
  merge(sourceimgcomplex, 2, sourceimgcomplexmerged);
  dft(sourceimgcomplexmerged, sourceimgcomplexmerged);
  return sourceimgcomplexmerged;
}

inline Mat ifourier(Mat&& img)
{
  Mat out;
  dft(img, out, DFT_INVERSE + DFT_SCALE + DFT_REAL_OUTPUT);
  return out;
}

inline Mat fourier(const Mat& sourceimgIn)
{
  Mat img = sourceimgIn.clone();
  return fourier(std::move(img));
}

inline Mat ifourier(const Mat& sourceimgIn)
{
  Mat img = sourceimgIn.clone();
  return ifourier(std::move(img));
}

inline Mat fourierinv(const Mat& realIn, const Mat& imagIn)
{
  Mat real = realIn.clone();
  Mat imag = imagIn.clone();
  Mat invDFT;
  Mat DFTcomplex[2] = {real, imag};
  Mat DFTcomplexmerged;
  merge(DFTcomplex, 2, DFTcomplexmerged);
  dft(DFTcomplexmerged, invDFT, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
  normalize(invDFT, invDFT, 0, 65535, NORM_MINMAX);
  invDFT.convertTo(invDFT, CV_16UC1);
  return invDFT;
}

inline Mat edgemask(int rows, int cols)
{
  Mat edgemask;
  createHanningWindow(edgemask, cv::Size(cols, rows), CV_32F);
  return edgemask;
}

inline Mat gaussian(int rows, int cols, double stdevYmult, double stdevXmult)
{
  Mat gaussian = Mat::zeros(rows, cols, CV_32F);
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
      gaussian.at<float>(r, c) = std::exp(-(std::pow(c - cols / 2, 2) / 2 / std::pow((double)cols / stdevXmult, 2) +
                                            std::pow(r - rows / 2, 2) / 2 / std::pow((double)rows / stdevYmult, 2)));

  normalize(gaussian, gaussian, 0, 1, NORM_MINMAX);
  return gaussian;
}

inline Mat laplacian(int rows, int cols, double stdevYmult, double stdevXmult)
{
  Mat laplacian = Mat::ones(rows, cols, CV_32F);
  laplacian = 1 - gaussian(rows, cols, stdevYmult, stdevXmult);
  normalize(laplacian, laplacian, 0, 1, NORM_MINMAX);
  return laplacian;
}

inline Mat bandpassian(int rows, int cols, double stdevLmult, double stdevHmult)
{
  Mat bandpassian = gaussian(rows, cols, stdevLmult, stdevLmult).mul(laplacian(rows, cols, 1. / stdevHmult, 1. / stdevHmult));
  normalize(bandpassian, bandpassian, 0, 1, NORM_MINMAX);
  return bandpassian;
}

inline Mat sinian(int rows, int cols, double frequencyX, double frequencyY)
{
  Mat sinian = Mat::zeros(rows, cols, CV_32F);
  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < cols; x++)
    {
      sinian.at<float>(y, x) = std::sin(2 * Constants::Pi * (y + x) * frequencyX); // sin or cos just cahnges the phase spectum
    }
  }
  normalize(sinian, sinian, 0, 1, NORM_MINMAX);
  sinian = sinian.mul(edgemask(rows, cols));
  return sinian;
}

inline Mat bandpass(const Mat& sourceimgDFTIn, const Mat& bandpassMat)
{
  Mat sourceimgDFT = sourceimgDFTIn.clone();
  Mat filterGS = quadrantswap(bandpassMat);
  Mat filter;
  Mat filterPlanes[2] = {filterGS, filterGS};
  merge(filterPlanes, 2, filter);
  return sourceimgDFT.mul(filter);
}

void showfourier(const Mat& DFTimgIn, bool logar = true, bool expon = false, std::string magnwindowname = "FFTmagn",
                 std::string phasewindowname = "FFTphase");

Mat convolute(Mat img, Mat PSFimg);

Mat deconvolute(Mat img, Mat PSFimg);

Mat deconvoluteWiener(const Mat& img, const Mat& PSFimg);

Mat frequencyFilter(const Mat& img, const Mat& mask);
