#pragma once
#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "Core/functionsBaseSTL.h"

using namespace std;
using namespace cv;
// using namespace cv::xfeatures2d;

inline Mat roicrop(const Mat &sourceimgIn, int x, int y, int w, int h)
{
  if (x < 0 || y < 0 || x - w / 2 < 0 || y - h / 2 < 0 || x + w / 2 > sourceimgIn.cols || y + h / 2 > sourceimgIn.rows)
    throw std::runtime_error("roicrop out of bounds");

  Rect roi = Rect(x - w / 2, y - h / 2, w, h);
  Mat crop = sourceimgIn(roi);
  return crop.clone();
}

inline double magnitude(const Point2f &pt) { return sqrt(sqr(pt.x) + sqr(pt.y)); }

inline double angle(const Point2f &pt) { return toDegrees(atan2(pt.y, pt.x)); }

inline std::pair<double, double> minMaxMat(const Mat &sourceimg)
{
  double minR, maxR;
  minMaxLoc(sourceimg, &minR, &maxR, nullptr, nullptr);
  return make_pair(minR, maxR);
}

inline std::string to_string(const Point2d &point) { return std::string("[" + to_string(point.x) + "," + to_string(point.y) + "]"); }

inline Point2f findCentroid(const Mat &sourceimg)
{
  double M = 0.0;
  double My = 0.0;
  double Mx = 0.0;
  for (int r = 0; r < sourceimg.rows; r++)
  {
    for (int c = 0; c < sourceimg.cols; c++)
    {
      M += sourceimg.at<float>(r, c);
      My += (double)r * sourceimg.at<float>(r, c);
      Mx += (double)c * sourceimg.at<float>(r, c);
    }
  }

  Point2f ret(Mx / M, My / M);

  if (ret.x < 0 || ret.y < 0 || ret.x > sourceimg.cols || ret.y > sourceimg.rows)
    return Point2f(sourceimg.cols / 2, sourceimg.rows / 2);
  else
    return ret;
}

inline Point2d median(std::vector<Point2d> &vec)
{
  // function changes the vec order, watch out
  std::sort(vec.begin(), vec.end(), [](Point2d a, Point2d b) { return a.x < b.x; });
  return vec[vec.size() / 2];
}

inline Point2f mean(const std::vector<Point2f> &vec)
{
  Point2f mean(0, 0);
  for (auto &x : vec)
    mean += x;
  return mean * (1. / vec.size());
}
