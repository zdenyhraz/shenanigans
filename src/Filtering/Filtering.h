#pragma once

inline cv::Mat filterContrastBrightness(const cv::Mat& sourceimg, f64 contrast, f64 brightness)
{
  cv::Mat filtered = sourceimg.clone();
  filtered.convertTo(filtered, CV_16U);
  normalize(filtered, filtered, 0, 65535, cv::NORM_MINMAX);
  for (i32 r = 0; r < filtered.rows; r++)
  {
    for (i32 c = 0; c < filtered.cols; c++)
    {
      if (filtered.channels() > 1)
      {
        filtered.at<cv::Vec3w>(r, c)[0] = clamp(contrast * (filtered.at<cv::Vec3w>(r, c)[0] + brightness), 0, 65535);
        filtered.at<cv::Vec3w>(r, c)[1] = clamp(contrast * (filtered.at<cv::Vec3w>(r, c)[1] + brightness), 0, 65535);
        filtered.at<cv::Vec3w>(r, c)[2] = clamp(contrast * (filtered.at<cv::Vec3w>(r, c)[2] + brightness), 0, 65535);
      }
      else
      {
        filtered.at<ushort>(r, c) = clamp(contrast * ((f64)filtered.at<ushort>(r, c) + brightness), 0, 65535);
      }
    }
  }
  normalize(filtered, filtered, 0, 65535, cv::NORM_MINMAX);
  return filtered;
}

inline cv::Mat histogramEqualize(const cv::Mat& sourceimgIn)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  normalize(sourceimg, sourceimg, 0, 255, cv::NORM_MINMAX);
  sourceimg.convertTo(sourceimg, CV_8U);
  equalizeHist(sourceimg, sourceimg);
  sourceimg.convertTo(sourceimg, CV_16U);
  normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);
  std::cout << "histogram equalized" << std::endl;
  return sourceimg;
}

inline cv::Mat gammaCorrect(const cv::Mat& sourceimgIn, f64 gamma) // returns CV_16UC1/3
{
  cv::Mat sourceimg = sourceimgIn.clone();

  sourceimg.convertTo(sourceimg, CV_32F);
  normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);
  pow(sourceimg, 1. / gamma, sourceimg);
  sourceimg.convertTo(sourceimg, CV_16U);
  normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);

  return sourceimg;
}

inline cv::Mat addnoise(const cv::Mat& sourceimgIn)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  sourceimg.convertTo(sourceimg, CV_32F);
  cv::Mat noised = cv::Mat::zeros(sourceimg.rows, sourceimg.cols, CV_32F);
  randn(noised, 0, 255 / 50);
  noised = noised + sourceimg;
  normalize(noised, noised, 0, 255, cv::NORM_MINMAX);
  noised.convertTo(noised, CV_8U);
  return noised;
}

inline void addnoise(cv::Mat& img, f64 stddev)
{
  std::random_device device;
  std::mt19937 generator(device());
  std::normal_distribution<f32> distribution(0, stddev);

  for (i32 c = 0; c < img.cols; c++)
    for (i32 r = 0; r < img.rows; r++)
      img.at<f32>(r, c) = std::clamp(img.at<f32>(r, c) + distribution(generator), 0.f, 1.f);
}

inline void showhistogram(const cv::Mat& sourceimgIn, i32 channels, i32 minimum, i32 maximum, std::string winname)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  /// Separate the image in 3 places ( B, G and R )
  std::vector<cv::Mat> bgr_planes(3);
  if (channels > 1)
    split(sourceimg, bgr_planes);
  else
  {
    bgr_planes[0] = sourceimg;
    bgr_planes[1] = sourceimg;
    bgr_planes[2] = sourceimg;
  }

  /// Establish the number of bins
  i32 histSize = 256;

  /// Set the ranges ( for B,G,R) )
  f32 range[] = {(f32)minimum, (f32)maximum};
  const f32* histRange = {range};

  bool uniform = true;
  bool accumulate = false;

  cv::Mat b_hist, g_hist, r_hist;

  /// Compute the histograms:
  calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
  calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
  calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

  // Draw the histograms for B, G and R
  i32 hist_w = 512;
  i32 hist_h = 400;
  i32 bin_w = cvRound((f64)hist_w / histSize);

  cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(b_hist, b_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
  normalize(g_hist, g_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
  normalize(r_hist, r_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

  /// Draw for each channel
  i32 thickness = 1;
  for (i32 i = 1; i < histSize; i++)
  {
    line(histImage, cv::Point2i(bin_w * (i - 1), hist_h - cvRound(b_hist.at<f32>(i - 1))), cv::Point2i(bin_w * (i), hist_h - cvRound(b_hist.at<f32>(i))), cv::Scalar(255, 0, 0), thickness, 8, 0);
    line(histImage, cv::Point2i(bin_w * (i - 1), hist_h - cvRound(g_hist.at<f32>(i - 1))), cv::Point2i(bin_w * (i), hist_h - cvRound(g_hist.at<f32>(i))), cv::Scalar(0, 255, 0), thickness, 8, 0);
    line(histImage, cv::Point2i(bin_w * (i - 1), hist_h - cvRound(r_hist.at<f32>(i - 1))), cv::Point2i(bin_w * (i), hist_h - cvRound(r_hist.at<f32>(i))), cv::Scalar(0, 0, 255), thickness, 8, 0);
  }

  /// Display
  showimg(histImage, winname);
}

template <typename T>
inline T GetMedian(const cv::Mat& mat)
{
  std::vector<T> values(mat.rows * mat.cols);
  for (i32 r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (i32 c = 0; c < mat.cols; ++c)
      values[r * mat.cols + c] = matp[c];
  }

  return Median(values);
}

template <typename T>
inline cv::Mat MedianBlur(const cv::Mat& mat, i32 kwidth, i32 kheight)
{
  if (kwidth % 2 == 0 or kheight % 2 == 0)
    [[unlikely]] throw std::invalid_argument("Invalid median blur window size");

  cv::Mat med = cv::Mat::zeros(mat.size(), GetMatType<T>());
  for (i32 r = 0; r < mat.rows; ++r)
  {
    auto medp = med.ptr<T>(r);
    for (i32 c = 0; c < mat.cols; ++c)
      medp[c] = GetMedian<T>(roicropclipref(mat, c, r, kwidth, kheight));
  }

  return med;
}