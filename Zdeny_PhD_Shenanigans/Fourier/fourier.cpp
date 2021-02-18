#include "stdafx.h"
#include "fourier.h"

#ifdef FOURIER_WITH_FFTW
#include <fftw3.h>
#endif

void showfourier(const Mat& DFTimgIn, bool logar, bool expon, std::string magnwindowname, std::string phasewindowname)
{
  Mat DFTimg = DFTimgIn.clone();
  if (DFTimg.channels() == 2)
  {
    Mat DFTimgcentered = quadrantswap(DFTimg);
    Mat planes[2];                 // = { Mat::zeros(DFTimg.size(),CV_32F),Mat::zeros(DFTimg.size(),CV_32F) };
    split(DFTimgcentered, planes); // planes[0] = Re(DFT(sourceimg), planes[1] = Im(DFT(sourceimg))
    Mat magnitudeimglog;
    magnitude(planes[0], planes[1], magnitudeimglog);
    if (logar)
    {
      magnitudeimglog += Scalar::all(1);
      log(magnitudeimglog, magnitudeimglog);
      normalize(magnitudeimglog, magnitudeimglog, 0, 1, NORM_MINMAX);
    }
    if (expon)
    {
      for (int i = 0; i < 20; i++)
      {
        exp(magnitudeimglog, magnitudeimglog);
        normalize(magnitudeimglog, magnitudeimglog, 0, 1, NORM_MINMAX);
      }
    }

    Plot2D::Plot(magnwindowname, magnitudeimglog);
    // showimg(magnitudeimglog, magnwindowname, true);

    // Mat phaseimg;
    // phase(planes[0], planes[1], phaseimg);
    // showimg(phaseimg, phasewindowname, true);
  }
  if (DFTimg.channels() == 1)
  {
    Mat DFTimgcentered = quadrantswap(DFTimg);
    if (logar)
    {
      DFTimgcentered = abs(DFTimgcentered);
      DFTimgcentered += Scalar::all(1);
      log(DFTimgcentered, DFTimgcentered);
      normalize(DFTimgcentered, DFTimgcentered, 0, 1, NORM_MINMAX);
    }
    showimg(DFTimgcentered, magnwindowname, true);
  }
}

Mat convolute(Mat sourceimg, Mat PSFimg)
{
  Mat DFT1 = fourier(sourceimg);
  Mat DFT2 = fourier(PSFimg);
  Mat planes1[2];
  Mat planes2[2];
  Mat planesCon[2];
  split(DFT1, planes1);
  split(DFT2, planes2);
  Mat a = planes1[0];
  Mat b = planes1[1];
  Mat c = planes2[0];
  Mat d = planes2[1];
  planesCon[0] = a.mul(c) - b.mul(d); // pointwise multiplication real
  planesCon[1] = a.mul(d) + b.mul(c); // imag
  Mat convimg = fourierinv(planesCon[0], planesCon[1]);
  convimg = quadrantswap(convimg);
  normalize(convimg, convimg, 0, 65535, NORM_MINMAX);
  convimg.convertTo(convimg, CV_16UC1);
  std::cout << "convolution calculated" << std::endl;
  return convimg;
}

Mat deconvolute(Mat sourceimg, Mat PSFimg)
{
  Mat DFT1 = fourier(sourceimg);
  Mat DFT2 = fourier(PSFimg);
  Mat planes1[2] = {Mat::zeros(DFT1.size(), CV_32F), Mat::zeros(DFT1.size(), CV_32F)};
  Mat planes2[2] = {Mat::zeros(DFT2.size(), CV_32F), Mat::zeros(DFT2.size(), CV_32F)};
  Mat planesDec[2] = {Mat::zeros(DFT2.size(), CV_32F), Mat::zeros(DFT2.size(), CV_32F)};
  split(DFT1, planes1);
  split(DFT2, planes2);
  Mat a = planes1[0];
  Mat b = planes1[1];
  Mat c = planes2[0];
  Mat d = planes2[1];
  Mat denom = Mat::zeros(a.size(), CV_32F);
  Mat csq = Mat::zeros(a.size(), CV_32F);
  Mat dsq = Mat::zeros(a.size(), CV_32F);
  pow(c, 2, csq);
  pow(d, 2, dsq);
  denom = csq + dsq;
  Mat magnitudedenom;
  magnitude(c, d, magnitudedenom);
  pow(magnitudedenom, 2, magnitudedenom);
  Mat scitanec1 = (a.mul(c)).mul(1 / denom);
  Mat scitanec2 = (b.mul(d)).mul(1 / denom);
  Mat scitanec3 = (b.mul(c)).mul(1 / denom);
  Mat scitanec4 = (a.mul(d)).mul(1 / denom);
  planesDec[0] = (scitanec1 + scitanec2); // pointwise division real
  planesDec[1] = (scitanec3 + scitanec4); // imag
  Mat deconvimg = fourierinv(planesDec[0], planesDec[1]);
  deconvimg = quadrantswap(deconvimg);
  normalize(deconvimg, deconvimg, 0, 65535, NORM_MINMAX);
  deconvimg.convertTo(deconvimg, CV_16UC1);
  std::cout << "Naive deconvolution calculated" << std::endl;
  return deconvimg;
}

Mat deconvoluteWiener(const Mat& sourceimg, const Mat& PSFimg)
{
  Mat DFT1 = fourier(sourceimg);
  Mat DFT2 = fourier(PSFimg);
  Mat planes1[2] = {Mat::zeros(DFT1.size(), CV_32F), Mat::zeros(DFT1.size(), CV_32F)};
  Mat planes2[2] = {Mat::zeros(DFT2.size(), CV_32F), Mat::zeros(DFT2.size(), CV_32F)};
  Mat planesDec[2] = {Mat::zeros(DFT2.size(), CV_32F), Mat::zeros(DFT2.size(), CV_32F)};
  split(DFT1, planes1);
  split(DFT2, planes2);
  Mat a = planes1[0];
  Mat b = planes1[1];
  Mat c = planes2[0];
  Mat d = planes2[1];
  Mat denom = Mat::zeros(a.size(), CV_32F);
  Mat csq = Mat::zeros(a.size(), CV_32F);
  Mat dsq = Mat::zeros(a.size(), CV_32F);
  pow(c, 2, csq);
  pow(d, 2, dsq);
  denom = csq + dsq;
  Mat magnitudedenom;
  magnitude(c, d, magnitudedenom);
  pow(magnitudedenom, 2, magnitudedenom);
  Mat SNR = Mat::zeros(DFT2.size(), CV_32F);
  SNR = Scalar::all(1.) / 1e-13;
  Mat dampingfactor = (magnitudedenom) / (magnitudedenom + 1 / SNR);
  Mat scitanec1 = (a.mul(c)).mul(1 / denom);
  Mat scitanec2 = (b.mul(d)).mul(1 / denom);
  Mat scitanec3 = (b.mul(c)).mul(1 / denom);
  Mat scitanec4 = (a.mul(d)).mul(1 / denom);
  planesDec[0] = (scitanec1 + scitanec2).mul(dampingfactor); // pointwise division real
  planesDec[1] = (scitanec3 + scitanec4).mul(dampingfactor); // imag
  Mat deconvimg = fourierinv(planesDec[0], planesDec[1]);
  deconvimg = quadrantswap(deconvimg);
  normalize(deconvimg, deconvimg, 0, 65535, NORM_MINMAX);
  deconvimg.convertTo(deconvimg, CV_16UC1);
  std::cout << "Wiener deconvolution calculated" << std::endl;
  return deconvimg;
}

Mat frequencyFilter(const Mat& sourceimg, const Mat& mask)
{
  Mat sourceimgDFT = fourier(sourceimg);
  Mat planesold[2];
  Mat planesnew[2];
  split(sourceimgDFT, planesold);
  Mat oldmagnitude;
  Mat oldphase;
  magnitude(planesold[0], planesold[1], oldmagnitude);
  phase(planesold[0], planesold[1], oldphase);
  Mat newmagnitude = oldmagnitude.mul(quadrantswap(mask));
  polarToCart(newmagnitude, oldphase, planesnew[0], planesnew[1]);
  Mat invDFTready;
  merge(planesnew, 2, invDFTready);
  showfourier(invDFTready);
  Mat newimg;
  dft(invDFTready, newimg, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
  normalize(newimg, newimg, 0, 65535, NORM_MINMAX);
  newimg.convertTo(newimg, CV_16UC1);
  return newimg;
}

#ifdef FOURIER_WITH_FFTW
Mat fourierFFTW(const Mat& sourceimgIn, int fftw)
{
  Mat sourceimg = sourceimgIn.clone();
  sourceimg.convertTo(sourceimg, CV_32F, 1. / 65535);
  sourceimg.reserve(sourceimg.rows * 2); // space for imaginary part
  if (fftw == 1)                         // fftw slowest version
  {
    int r, c;
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * sourceimg.rows * sourceimg.cols);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * sourceimg.rows * sourceimg.cols);
    fftw_plan plan = fftw_plan_dft_2d(sourceimg.rows, sourceimg.cols, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    for (r = 0; r < sourceimg.rows; r++)
    {
      for (c = 0; c < sourceimg.cols; c++)
      {
        in[r * sourceimg.cols + c][0] = sourceimg.at<float>(r, c);
        in[r * sourceimg.cols + c][1] = 0;
      }
    }
    fftw_execute(plan);
    Mat result2[2] = {Mat::zeros(sourceimg.rows, sourceimg.cols, CV_32F), Mat::zeros(sourceimg.rows, sourceimg.cols, CV_32F)};
    for (r = 0; r < sourceimg.rows; r++)
    {
      for (c = 0; c < sourceimg.cols; c++)
      {
        result2[0].at<float>(r, c) = out[r * sourceimg.cols + c][0];
        result2[1].at<float>(r, c) = out[r * sourceimg.cols + c][1];
      }
    }
    Mat result;
    merge(result2, 2, result);
    fftw_free(in);
    fftw_free(out);
    fftw_destroy_plan(plan);
    return result;
  }
  if (fftw == 2)
  {
    fftw_plan plan = fftw_plan_dft_r2c_2d(sourceimg.rows, sourceimg.cols, (double*)sourceimg.data, (fftw_complex*)sourceimg.data, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    Mat resultRe = Mat(sourceimg.rows, sourceimg.cols, CV_32F, (double*)sourceimg.data, sourceimg.cols * sizeof(fftw_complex));
    Mat resultIm = Mat(sourceimg.rows, sourceimg.cols, CV_32F, (double*)sourceimg.data + 1, sourceimg.cols * sizeof(fftw_complex));
    Mat result2[2] = {resultRe, resultIm};
    Mat result;
    merge(result2, 2, result);
    return result;
  }
  if (fftw == 3)
  {
    fftw_plan plan = fftw_plan_dft_2d(sourceimg.rows, sourceimg.cols, (fftw_complex*)sourceimg.data, (fftw_complex*)sourceimg.data, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    Mat resultRe = Mat(sourceimg.rows, sourceimg.cols, CV_32F, (double*)sourceimg.data, sourceimg.cols * sizeof(fftw_complex));
    Mat resultIm = Mat(sourceimg.rows, sourceimg.cols, CV_32F, (double*)sourceimg.data + 1, sourceimg.cols * sizeof(fftw_complex));
    Mat result2[2] = {resultRe, resultIm};
    Mat result;
    merge(result2, 2, result);
    return result;
  }
}
#endif
