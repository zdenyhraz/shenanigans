#pragma once
#include "stdafx.h"

std::tuple<Mat, Mat> CalculateGradient(const Mat& img)
{
  Mat gradM = Mat::zeros(img.size(), CV_32F);
  Mat gradA = Mat::zeros(img.size(), CV_32F);

  for (int r = 1; r < img.rows - 1; ++r)
  {
    for (int c = 1; c < img.cols - 1; ++c)
    {
      float gradX = (img.at<float>(r, c + 1) - img.at<float>(r, c - 1)) / 2;
      float gradY = (img.at<float>(r + 1, c) - img.at<float>(r - 1, c)) / 2;
      gradM.at<float>(r, c) = sqrt(sqr(gradX) + sqr(gradY));
      float angle = atan2(gradY, gradX);
      gradA.at<float>(r, c) = angle >= 0 ? angle : angle + Constants::TwoPi;
    }
  }
  return {gradM, gradA};
}

std::tuple<int, int, int, int> GetTwoPairs(float angle, int r, int c, const std::vector<Point2f>& relativePoints)
{
  float mind = std::numeric_limits<float>::max();
  float mindangle = 0;
  int r1 = r, c1 = c, r2 = r, c2 = c;

  for (const auto& relativePoint : relativePoints)
  {
    float d = abs(sin(angle) * relativePoint.x - cos(angle) * relativePoint.y);

    // the closest point on the line
    Point2f closest;
    closest.x = -cos(angle) * (-cos(angle) * relativePoint.x - sin(angle) * relativePoint.y);
    closest.y = sin(angle) * (cos(angle) * relativePoint.x + sin(angle) * relativePoint.y);

    float dangle;
    dangle = atan2(relativePoint.y, relativePoint.x);                         // from the origin
    dangle = atan2(closest.y - relativePoint.y, closest.x - relativePoint.x); // from the closest point on the line
    dangle = dangle >= 0 ? dangle : dangle + Constants::TwoPi;

    if (d <= mind)
    {
      if (d == mind && dangle < mindangle)
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

Mat NonMaximaSuppresion(const Mat& img)
{
  auto [gradM, gradA] = CalculateGradient(img);
  Mat out = Mat::zeros(gradM.size(), CV_32F);

  int size = 3;
  std::vector<Point2f> relativePoints;
  for (int rr = -size / 2; rr <= size / 2; ++rr)
    for (int cc = -size / 2; cc <= size / 2; ++cc)
      if (rr || cc)
        relativePoints.push_back(Point2f(cc, rr));

  if (relativePoints.size() != sqr(size) - 1)
    throw;

  for (int r = 0; r < out.rows; ++r)
  {
    for (int c = 0; c < out.cols; ++c)
    {
      auto [r1, c1, r2, c2] = GetTwoPairs(gradA.at<float>(r, c), r, c, relativePoints);

      if ((gradM.at<float>(r1, c1) < gradM.at<float>(r, c)) && (gradM.at<float>(r2, c2) < gradM.at<float>(r, c)))
        out.at<float>(r, c) = gradM.at<float>(r, c);
    }
  }

  Plot2D::plot(img, "img");
  Plot2D::plot(gradM, "gradM");
  Plot2D::plot(gradA, "gradA");
  Plot2D::plot(out, "out");

  return out;
}
