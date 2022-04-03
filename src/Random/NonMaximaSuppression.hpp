#pragma once

inline std::tuple<cv::Mat, cv::Mat> CalculateGradient(const cv::Mat& img)
{
  cv::Mat gradM = cv::Mat::zeros(img.size(), CV_32F);
  cv::Mat gradA = cv::Mat::zeros(img.size(), CV_32F);

  for (i32 r = 1; r < img.rows - 1; ++r)
  {
    for (i32 c = 1; c < img.cols - 1; ++c)
    {
      f32 gradX = (img.at<f32>(r, c + 1) - img.at<f32>(r, c - 1)) / 2;
      f32 gradY = (img.at<f32>(r + 1, c) - img.at<f32>(r - 1, c)) / 2;
      gradM.at<f32>(r, c) = sqrt(Sqr(gradX) + Sqr(gradY));
      f32 angle = atan2(gradY, gradX);
      gradA.at<f32>(r, c) = angle >= 0 ? angle : angle + TwoPi;
    }
  }
  return {gradM, gradA};
}

inline std::tuple<i32, i32, i32, i32> GetTwoPairs(f32 angle, i32 r, i32 c, const std::vector<cv::Point2f>& relativePoints)
{
  f32 mind = std::numeric_limits<f32>::max();
  f32 mindangle = 0;
  i32 r1 = r, c1 = c, r2 = r, c2 = c;

  for (const auto& relativePoint : relativePoints)
  {
    f32 d = abs(sin(angle) * relativePoint.x - cos(angle) * relativePoint.y);

    // the closest point on the line
    cv::Point2f closest;
    closest.x = -cos(angle) * (-cos(angle) * relativePoint.x - sin(angle) * relativePoint.y);
    closest.y = sin(angle) * (cos(angle) * relativePoint.x + sin(angle) * relativePoint.y);

    f32 dangle;
    dangle = atan2(relativePoint.y, relativePoint.x);                         // from the origin
    dangle = atan2(closest.y - relativePoint.y, closest.x - relativePoint.x); // from the closest point on the line
    dangle = dangle >= 0 ? dangle : dangle + TwoPi;

    if (d <= mind)
    {
      if (d == mind and dangle < mindangle)
        continue;

      mind = d;
      mindangle = dangle;

      r1 = r + relativePoint.y;
      c1 = c + relativePoint.x;

      r2 = r - relativePoint.y;
      c2 = c - relativePoint.x;
    }
  }

  return {r1, c1, r2, c2};
}

inline cv::Mat NonMaximaSuppresion(const cv::Mat& img)
{
  auto [gradM, gradA] = CalculateGradient(img);
  cv::Mat out = cv::Mat::zeros(gradM.size(), CV_32F);

  i32 size = 3;
  std::vector<cv::Point2f> relativePoints;
  for (i32 rr = -size / 2; rr <= size / 2; ++rr)
    for (i32 cc = -size / 2; cc <= size / 2; ++cc)
      if (rr or cc)
        relativePoints.push_back(cv::Point2f(cc, rr));

  if (relativePoints.size() != static_cast<usize>(Sqr(size)) - 1)
    throw;

  for (i32 r = 1; r < out.rows - 1; ++r)
  {
    for (i32 c = 1; c < out.cols - 1; ++c)
    {
      auto [r1, c1, r2, c2] = GetTwoPairs(gradA.at<f32>(r, c), r, c, relativePoints);

      if (gradM.at<f32>(r1, c1) < gradM.at<f32>(r, c) and gradM.at<f32>(r2, c2) < gradM.at<f32>(r, c))
        out.at<f32>(r, c) = gradM.at<f32>(r, c);
    }
  }

  // Plot2D::Plot(img, "img");
  // Plot2D::Plot(gradM, "gradM");
  // Plot2D::Plot(gradA, "gradA");
  // Plot2D::Plot(out, "out");

  return out;
}
