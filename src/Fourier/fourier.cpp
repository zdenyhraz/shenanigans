
#include "UtilsCV/Showsave.h"
#include "Fourier.h"
#include "Plot/Plot2D.h"

void showfourier(const cv::Mat& DFTimgIn, bool logar, bool expon, const std::string& magnwindowname, const std::string& phasewindowname)
{
  cv::Mat DFTimg = DFTimgIn.clone();
  if (DFTimg.channels() == 2)
  {
    cv::Mat DFTimgcentered = quadrantswap(DFTimg);
    cv::Mat planes[2];             // = { cv::Mat::zeros(DFTimg.size(),CV_32F),cv::Mat::zeros(DFTimg.size(),CV_32F) };
    split(DFTimgcentered, planes); // planes[0] = Re(DFT(sourceimg), planes[1] = Im(DFT(sourceimg))
    cv::Mat magnitudeimglog;
    magnitude(planes[0], planes[1], magnitudeimglog);
    if (logar)
    {
      magnitudeimglog += cv::Scalar::all(1);
      log(magnitudeimglog, magnitudeimglog);
      normalize(magnitudeimglog, magnitudeimglog, 0, 1, cv::NORM_MINMAX);
    }
    if (expon)
    {
      for (usize i = 0; i < 20; i++)
      {
        exp(magnitudeimglog, magnitudeimglog);
        normalize(magnitudeimglog, magnitudeimglog, 0, 1, cv::NORM_MINMAX);
      }
    }

    Plot2D::Plot(magnwindowname, magnitudeimglog);
    // showimg(magnitudeimglog, magnwindowname, true);

    // cv::Mat phaseimg;
    // phase(planes[0], planes[1], phaseimg);
    // showimg(phaseimg, phasewindowname, true);
  }
  if (DFTimg.channels() == 1)
  {
    cv::Mat DFTimgcentered = quadrantswap(DFTimg);
    if (logar)
    {
      DFTimgcentered = abs(DFTimgcentered);
      DFTimgcentered += cv::Scalar::all(1);
      log(DFTimgcentered, DFTimgcentered);
      normalize(DFTimgcentered, DFTimgcentered, 0, 1, cv::NORM_MINMAX);
    }
    showimg(DFTimgcentered, magnwindowname, true);
  }
}

cv::Mat convolute(cv::Mat sourceimg, cv::Mat PSFimg)
{
  cv::Mat DFT1 = fourier(sourceimg);
  cv::Mat DFT2 = fourier(PSFimg);
  cv::Mat planes1[2];
  cv::Mat planes2[2];
  cv::Mat planesCon[2];
  split(DFT1, planes1);
  split(DFT2, planes2);
  cv::Mat a = planes1[0];
  cv::Mat b = planes1[1];
  cv::Mat c = planes2[0];
  cv::Mat d = planes2[1];
  planesCon[0] = a.mul(c) - b.mul(d); // pointwise multiplication real
  planesCon[1] = a.mul(d) + b.mul(c); // imag
  cv::Mat convimg = fourierinv(planesCon[0], planesCon[1]);
  convimg = quadrantswap(convimg);
  normalize(convimg, convimg, 0, 65535, cv::NORM_MINMAX);
  convimg.convertTo(convimg, CV_16UC1);
  std::cout << "convolution calculated" << std::endl;
  return convimg;
}

cv::Mat deconvolute(cv::Mat sourceimg, cv::Mat PSFimg)
{
  cv::Mat DFT1 = fourier(sourceimg);
  cv::Mat DFT2 = fourier(PSFimg);
  cv::Mat planes1[2] = {cv::Mat::zeros(DFT1.size(), CV_32F), cv::Mat::zeros(DFT1.size(), CV_32F)};
  cv::Mat planes2[2] = {cv::Mat::zeros(DFT2.size(), CV_32F), cv::Mat::zeros(DFT2.size(), CV_32F)};
  cv::Mat planesDec[2] = {cv::Mat::zeros(DFT2.size(), CV_32F), cv::Mat::zeros(DFT2.size(), CV_32F)};
  split(DFT1, planes1);
  split(DFT2, planes2);
  cv::Mat a = planes1[0];
  cv::Mat b = planes1[1];
  cv::Mat c = planes2[0];
  cv::Mat d = planes2[1];
  cv::Mat denom = cv::Mat::zeros(a.size(), CV_32F);
  cv::Mat csq = cv::Mat::zeros(a.size(), CV_32F);
  cv::Mat dsq = cv::Mat::zeros(a.size(), CV_32F);
  pow(c, 2, csq);
  pow(d, 2, dsq);
  denom = csq + dsq;
  cv::Mat magnitudedenom;
  magnitude(c, d, magnitudedenom);
  pow(magnitudedenom, 2, magnitudedenom);
  cv::Mat scitanec1 = (a.mul(c)).mul(1 / denom);
  cv::Mat scitanec2 = (b.mul(d)).mul(1 / denom);
  cv::Mat scitanec3 = (b.mul(c)).mul(1 / denom);
  cv::Mat scitanec4 = (a.mul(d)).mul(1 / denom);
  planesDec[0] = (scitanec1 + scitanec2); // pointwise division real
  planesDec[1] = (scitanec3 + scitanec4); // imag
  cv::Mat deconvimg = fourierinv(planesDec[0], planesDec[1]);
  deconvimg = quadrantswap(deconvimg);
  normalize(deconvimg, deconvimg, 0, 65535, cv::NORM_MINMAX);
  deconvimg.convertTo(deconvimg, CV_16UC1);
  std::cout << "Naive deconvolution calculated" << std::endl;
  return deconvimg;
}

cv::Mat deconvoluteWiener(const cv::Mat& sourceimg, const cv::Mat& PSFimg)
{
  cv::Mat DFT1 = fourier(sourceimg);
  cv::Mat DFT2 = fourier(PSFimg);
  cv::Mat planes1[2] = {cv::Mat::zeros(DFT1.size(), CV_32F), cv::Mat::zeros(DFT1.size(), CV_32F)};
  cv::Mat planes2[2] = {cv::Mat::zeros(DFT2.size(), CV_32F), cv::Mat::zeros(DFT2.size(), CV_32F)};
  cv::Mat planesDec[2] = {cv::Mat::zeros(DFT2.size(), CV_32F), cv::Mat::zeros(DFT2.size(), CV_32F)};
  split(DFT1, planes1);
  split(DFT2, planes2);
  cv::Mat a = planes1[0];
  cv::Mat b = planes1[1];
  cv::Mat c = planes2[0];
  cv::Mat d = planes2[1];
  cv::Mat denom = cv::Mat::zeros(a.size(), CV_32F);
  cv::Mat csq = cv::Mat::zeros(a.size(), CV_32F);
  cv::Mat dsq = cv::Mat::zeros(a.size(), CV_32F);
  pow(c, 2, csq);
  pow(d, 2, dsq);
  denom = csq + dsq;
  cv::Mat magnitudedenom;
  magnitude(c, d, magnitudedenom);
  pow(magnitudedenom, 2, magnitudedenom);
  cv::Mat SNR = cv::Mat::zeros(DFT2.size(), CV_32F);
  SNR = cv::Scalar::all(1.) / 1e-13;
  cv::Mat dampingfactor = (magnitudedenom) / (magnitudedenom + 1 / SNR);
  cv::Mat scitanec1 = (a.mul(c)).mul(1 / denom);
  cv::Mat scitanec2 = (b.mul(d)).mul(1 / denom);
  cv::Mat scitanec3 = (b.mul(c)).mul(1 / denom);
  cv::Mat scitanec4 = (a.mul(d)).mul(1 / denom);
  planesDec[0] = (scitanec1 + scitanec2).mul(dampingfactor); // pointwise division real
  planesDec[1] = (scitanec3 + scitanec4).mul(dampingfactor); // imag
  cv::Mat deconvimg = fourierinv(planesDec[0], planesDec[1]);
  deconvimg = quadrantswap(deconvimg);
  normalize(deconvimg, deconvimg, 0, 65535, cv::NORM_MINMAX);
  deconvimg.convertTo(deconvimg, CV_16UC1);
  std::cout << "Wiener deconvolution calculated" << std::endl;
  return deconvimg;
}

cv::Mat frequencyFilter(const cv::Mat& sourceimg, const cv::Mat& mask)
{
  cv::Mat sourceimgDFT = fourier(sourceimg);
  cv::Mat planesold[2];
  cv::Mat planesnew[2];
  split(sourceimgDFT, planesold);
  cv::Mat oldmagnitude;
  cv::Mat oldphase;
  magnitude(planesold[0], planesold[1], oldmagnitude);
  phase(planesold[0], planesold[1], oldphase);
  cv::Mat newmagnitude = oldmagnitude.mul(quadrantswap(mask));
  polarToCart(newmagnitude, oldphase, planesnew[0], planesnew[1]);
  cv::Mat invDFTready;
  merge(planesnew, 2, invDFTready);
  showfourier(invDFTready);
  cv::Mat newimg;
  dft(invDFTready, newimg, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
  normalize(newimg, newimg, 0, 65535, cv::NORM_MINMAX);
  newimg.convertTo(newimg, CV_16UC1);
  return newimg;
}

#ifdef FOURIER_WITH_FFTW
cv::Mat fourierFFTW(const cv::Mat& sourceimgIn, i32 fftw)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  sourceimg.convertTo(sourceimg, CV_32F, 1. / 65535);
  sourceimg.reserve(sourceimg.rows * 2); // space for imaginary part
  if (fftw == 1)                         // fftw slowest version
  {
    i32 r, c;
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * sourceimg.rows * sourceimg.cols);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * sourceimg.rows * sourceimg.cols);
    fftw_plan plan = fftw_plan_dft_2d(sourceimg.rows, sourceimg.cols, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    for (r = 0; r < sourceimg.rows; r++)
    {
      for (c = 0; c < sourceimg.cols; c++)
      {
        in[r * sourceimg.cols + c][0] = sourceimg.at<f32>(r, c);
        in[r * sourceimg.cols + c][1] = 0;
      }
    }
    fftw_execute(plan);
    cv::Mat result2[2] = {cv::Mat::zeros(sourceimg.rows, sourceimg.cols, CV_32F), cv::Mat::zeros(sourceimg.rows, sourceimg.cols, CV_32F)};
    for (r = 0; r < sourceimg.rows; r++)
    {
      for (c = 0; c < sourceimg.cols; c++)
      {
        result2[0].at<f32>(r, c) = out[r * sourceimg.cols + c][0];
        result2[1].at<f32>(r, c) = out[r * sourceimg.cols + c][1];
      }
    }
    cv::Mat result;
    merge(result2, 2, result);
    fftw_free(in);
    fftw_free(out);
    fftw_destroy_plan(plan);
    return result;
  }
  if (fftw == 2)
  {
    fftw_plan plan = fftw_plan_dft_r2c_2d(sourceimg.rows, sourceimg.cols, (f64*)sourceimg.data, (fftw_complex*)sourceimg.data, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    cv::Mat resultRe = cv::Mat(sourceimg.rows, sourceimg.cols, CV_32F, (f64*)sourceimg.data, sourceimg.cols * sizeof(fftw_complex));
    cv::Mat resultIm = cv::Mat(sourceimg.rows, sourceimg.cols, CV_32F, (f64*)sourceimg.data + 1, sourceimg.cols * sizeof(fftw_complex));
    cv::Mat result2[2] = {resultRe, resultIm};
    cv::Mat result;
    merge(result2, 2, result);
    return result;
  }
  if (fftw == 3)
  {
    fftw_plan plan = fftw_plan_dft_2d(sourceimg.rows, sourceimg.cols, (fftw_complex*)sourceimg.data, (fftw_complex*)sourceimg.data, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    cv::Mat resultRe = cv::Mat(sourceimg.rows, sourceimg.cols, CV_32F, (f64*)sourceimg.data, sourceimg.cols * sizeof(fftw_complex));
    cv::Mat resultIm = cv::Mat(sourceimg.rows, sourceimg.cols, CV_32F, (f64*)sourceimg.data + 1, sourceimg.cols * sizeof(fftw_complex));
    cv::Mat result2[2] = {resultRe, resultIm};
    cv::Mat result;
    merge(result2, 2, result);
    return result;
  }
}
#endif
