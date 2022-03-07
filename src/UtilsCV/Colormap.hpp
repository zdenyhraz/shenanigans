#pragma once

inline cv::Scalar ColormapJet(f32 x, f32 caxisMin = 0, f32 caxisMax = 1, f32 val = 255)
{
  f32 B, G, R;
  f32 sh = 0.125 * (caxisMax - caxisMin);
  f32 start = caxisMin;
  f32 mid = caxisMin + 0.5 * (caxisMax - caxisMin);
  f32 end = caxisMax;

  B = (x > (start + sh)) ? std::clamp(-val / 2 / sh * x + val / 2 / sh * (mid + sh), 0.f, val) : (x < start ? val / 2 : std::clamp(val / 2 / sh * x + val / 2 - val / 2 / sh * start, 0.f, val));
  G = (x < mid) ? std::clamp(val / 2 / sh * x - val / 2 / sh * (start + sh), 0.f, val) : std::clamp(-val / 2 / sh * x + val / 2 / sh * (end - sh), 0.f, val);
  R = (x < (end - sh)) ? std::clamp(val / 2 / sh * x - val / 2 / sh * (mid - sh), 0.f, val) : (x > end ? val / 2 : std::clamp(-val / 2 / sh * x + val / 2 + val / 2 / sh * end, 0.f, val));

  return cv::Scalar(B, G, R);
}

inline cv::Mat ApplyQuantile(const cv::Mat& sourceimgIn, f64 quantileB = 0, f64 quantileT = 1)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  sourceimg.convertTo(sourceimg, CV_32FC1);
  f32 caxisMin, caxisMax;
  std::tie(caxisMin, caxisMax) = MinMax(sourceimg);

  if (quantileB > 0 or quantileT < 1)
  {
    std::vector<f32> picvalues(sourceimg.rows * sourceimg.cols, 0);
    for (i32 r = 0; r < sourceimg.rows; r++)
      for (i32 c = 0; c < sourceimg.cols; c++)
        picvalues[r * sourceimg.cols + c] = sourceimg.at<f32>(r, c);

    sort(picvalues.begin(), picvalues.end());
    caxisMin = picvalues[quantileB * (picvalues.size() - 1)];
    caxisMax = picvalues[quantileT * (picvalues.size() - 1)];
  }

  for (i32 r = 0; r < sourceimg.rows; r++)
    for (i32 c = 0; c < sourceimg.cols; c++)
      sourceimg.at<f32>(r, c) = std::clamp(sourceimg.at<f32>(r, c), caxisMin, caxisMax);

  return sourceimg;
}

inline cv::Mat ApplyQuantileColormap(const cv::Mat& sourceimgIn, f64 quantileB = 0, f64 quantileT = 1)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  sourceimg.convertTo(sourceimg, CV_32F);
  f32 caxisMin, caxisMax;
  std::tie(caxisMin, caxisMax) = MinMax(sourceimg);

  if (quantileB > 0 or quantileT < 1)
  {
    std::vector<f32> picvalues(sourceimg.rows * sourceimg.cols, 0);
    for (i32 r = 0; r < sourceimg.rows; r++)
      for (i32 c = 0; c < sourceimg.cols; c++)
        picvalues[r * sourceimg.cols + c] = sourceimg.at<f32>(r, c);

    sort(picvalues.begin(), picvalues.end());
    caxisMin = picvalues[quantileB * (picvalues.size() - 1)];
    caxisMax = picvalues[quantileT * (picvalues.size() - 1)];
  }

  cv::Mat sourceimgOutCLR(sourceimg.rows, sourceimg.cols, CV_32FC3);
  for (i32 r = 0; r < sourceimgOutCLR.rows; r++)
  {
    for (i32 c = 0; c < sourceimgOutCLR.cols; c++)
    {
      f32 x = sourceimg.at<f32>(r, c);
      const auto clr = ColormapJet(x, caxisMin, caxisMax);
      sourceimgOutCLR.at<cv::Vec3f>(r, c)[0] = clr[0];
      sourceimgOutCLR.at<cv::Vec3f>(r, c)[1] = clr[1];
      sourceimgOutCLR.at<cv::Vec3f>(r, c)[2] = clr[2];
    }
  }
  return sourceimgOutCLR;
}
