#pragma once

struct Object
{
  cv::Point center;
  std::vector<cv::Point> contour;
};

inline cv::Mat CalculateObjectness(const cv::Mat& edges, i32 objectSize = 50)
{
  LOG_FUNCTION;
  cv::Mat edgesNorm(edges.size(), CV_32F);
  cv::normalize(edges, edgesNorm, 0, 1, cv::NORM_MINMAX);

  cv::Mat objectness = cv::Mat::zeros(edgesNorm.size(), CV_32F);
  for (i32 r = objectSize / 2; r < edgesNorm.rows - objectSize / 2; ++r)
    for (i32 c = objectSize / 2; c < edgesNorm.cols - objectSize / 2; ++c)
      objectness.at<f32>(r, c) = cv::sum(RoiCropRef(edgesNorm, c, r, objectSize, objectSize))[0] / (objectSize * objectSize); // pixel average

  return objectness;
}

inline std::vector<Object> CalculateObjects(const cv::Mat& objectness, f32 objectThreshold)
{
  LOG_FUNCTION;
  cv::Mat objectness8U = objectness.clone();
  cv::threshold(objectness8U, objectness8U, objectThreshold, 255, cv::THRESH_BINARY);
  objectness8U.convertTo(objectness8U, CV_8U);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/objectness_thr.png").string(), objectness8U, false, {0, 0}, true);

  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(objectness8U, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
  LOG_DEBUG("Found {} objects", contours.size());

  std::vector<Object> objects;
  const auto minWidth = 0;
  const auto minHeight = 0;
  for (const auto& contour : contours)
  {
    if (const auto minRect = cv::minAreaRect(contour); minRect.size.width < minWidth and minRect.size.height < minHeight)
      continue;

    const auto center = std::accumulate(contour.begin(), contour.end(), cv::Point{0, 0}) / static_cast<i32>(contour.size());
    objects.emplace_back(center, contour);
  }

  return objects;
}

inline cv::Mat DrawObjects(const cv::Mat& source, const std::vector<Object>& objects)
{
  LOG_FUNCTION;
  cv::Mat out(source.size(), source.type());
  cv::cvtColor(source, out, cv::COLOR_GRAY2BGR);

  const auto color = cv::Scalar(0, 0, 255);
  const auto thickness = std::clamp(out.cols / 100, 1, 5);

  for (const auto& object : objects)
  {
    // DrawPoint(out, object.center, color, 0.01, thickness);
    cv::drawContours(out, std::vector<std::vector<cv::Point>>{object.contour}, -1, color, thickness, cv::LINE_AA);
  }

  return out;
}

inline void DetectObjectsStddev(const cv::Mat& source, i32 objectSize, f32 objectThreshold, i32 blurSize, i32 stddevSize)
{
  LOG_FUNCTION;
  cv::Mat blurred(source.size(), source.type());
  if (0)
    cv::GaussianBlur(source, blurred, cv::Size(blurSize, blurSize), 0, 0);
  else
    cv::medianBlur(source, blurred, blurSize);

  cv::Mat stddevs = cv::Mat::zeros(blurred.size(), CV_32F);
  for (i32 r = stddevSize / 2; r < blurred.rows - stddevSize / 2; ++r)
  {
    for (i32 c = stddevSize / 2; c < blurred.cols - stddevSize / 2; ++c)
    {
      cv::Scalar mean, stddev;
      cv::meanStdDev(RoiCropRef(blurred, c, r, stddevSize, stddevSize), mean, stddev);
      stddevs.at<f32>(r, c) = stddev[0];
    }
  }
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/stddevs.png").string(), stddevs, false, {0, 0}, true);

  f32 minThreshold = 0.1; // stddev needs to be at least 0.1 for object pixels
  cv::threshold(stddevs, stddevs, minThreshold * 255, 1, cv::THRESH_BINARY);

  const auto objectness = CalculateObjectness(stddevs, objectSize);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/objectness.png").string(), objectness);

  const auto objects = CalculateObjects(objectness, objectThreshold);

  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/blurred.png").string(), blurred);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/stddevs_thr.png").string(), stddevs, false, {0, 0}, true);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/objectness.png").string(), objectness, false, {0, 0}, true);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/objects.png").string(), DrawObjects(source, objects));
}

inline void DetectObjectsCanny(
    const cv::Mat& source, i32 objectSize = 50, f32 objectThreshold = 0.03, i32 blurSize = 11, i32 sobelSize = 3, f32 lowThreshold = 0.3, f32 highThreshold = 0.9)
{
  LOG_FUNCTION;
  cv::Mat blurred(source.size(), source.type());
  if (0)
    cv::GaussianBlur(source, blurred, cv::Size(blurSize, blurSize), 0, 0);
  else
    cv::medianBlur(source, blurred, blurSize);

  cv::Mat canny;
  cv::Canny(blurred, canny, lowThreshold * 255, highThreshold * 255, sobelSize);

  const auto objectness = CalculateObjectness(canny, objectSize);
  const auto objects = CalculateObjects(objectness, objectThreshold);
}
