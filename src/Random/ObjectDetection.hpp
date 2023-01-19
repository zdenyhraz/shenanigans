#pragma once

struct Object
{
  cv::Point center;
  std::vector<cv::Point> contour;
};

inline cv::Mat CalculateObjectness(const cv::Mat& edges, i32 objectSize)
{
  LOG_FUNCTION;
  cv::Mat edgesNorm(edges.size(), CV_32F);
  cv::normalize(edges, edgesNorm, 0, 1, cv::NORM_MINMAX);

  cv::Mat objectness = cv::Mat::zeros(edgesNorm.size(), CV_32F);
  cv::Mat window = Kirkl<f32>(objectSize);
#pragma omp parallel for
  for (i32 r = objectSize / 2; r < edgesNorm.rows - objectSize / 2; ++r)
    for (i32 c = objectSize / 2; c < edgesNorm.cols - objectSize / 2; ++c)
      objectness.at<f32>(r, c) = cv::sum(RoiCropRef(edgesNorm, c, r, objectSize, objectSize).mul(window))[0];

  return objectness / (objectSize * objectSize); // pixel average
}

inline std::vector<Object> CalculateObjects(const cv::Mat& objectness, f32 objectnessThreshold, f32 minObjectSize)
{
  LOG_FUNCTION;
  cv::Mat objectness8U = objectness.clone();
  cv::threshold(objectness8U, objectness8U, objectnessThreshold, 255, cv::THRESH_BINARY);
  objectness8U.convertTo(objectness8U, CV_8U);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/objectness_thr.png").string(), objectness8U, false, {0, 0}, true);

  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(objectness8U, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
  LOG_DEBUG("Found {} objects", contours.size());

  std::vector<Object> objects;
  for (const auto& contour : contours)
  {
    // filter out small objects
    if (const auto minRect = cv::minAreaRect(contour); minRect.size.width < minObjectSize and minRect.size.height < minObjectSize)
      continue;

    // filter out "needle" artifacts
    if (const auto minRect = cv::minAreaRect(contour); minRect.size.width < minObjectSize or minRect.size.height < minObjectSize)
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

  const auto color = cv::Scalar(0, 0, 1);
  const auto thickness = std::clamp(0.005 * out.rows, 1., 100.);
  for (const auto& object : objects)
    cv::drawContours(out, std::vector<std::vector<cv::Point>>{object.contour}, -1, color, thickness, cv::LINE_AA);

  return out;
}

inline void DetectObjectsEdge(const cv::Mat& source)
{
  LOG_FUNCTION;
  const auto objectSize = 0.05 * source.rows;                       // 0.02
  const auto blurSizeTarget = static_cast<i32>(0.05 * source.rows); // 0.005
  const auto blurSize = blurSizeTarget % 2 == 0 ? blurSizeTarget + 1 : blurSizeTarget;
  const auto edgeSizeTarget = static_cast<i32>(0.0025 * source.rows); // 0.0025
  const auto edgeSize = edgeSizeTarget % 2 == 0 ? edgeSizeTarget + 1 : edgeSizeTarget;
  const auto minObjectSize = 0.02 * source.rows; // 0.02
  const auto edgeThreshold = 0.7;                // 0.3 relative normalized
  const auto objectnessThreshold = 0.01;         // 0.01

  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/source.png").string(), source);
  cv::Mat blurred(source.size(), source.type());
  cv::GaussianBlur(source, blurred, cv::Size(blurSize, blurSize), 0);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/blurred.png").string(), blurred);
  cv::Mat edges;
  cv::Sobel(blurred, edges, CV_32F, 1, 0, edgeSize, 1, 0, cv::BORDER_REPLICATE);
  edges = cv::abs(edges);
  cv::normalize(edges, edges, 0, 1, cv::NORM_MINMAX);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/edges.png").string(), edges, false, {0, 0}, true);
  cv::threshold(edges, edges, edgeThreshold, 1, cv::THRESH_BINARY);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/edges_thr.png").string(), edges, false, {0, 0}, true);
  const auto objectness = CalculateObjectness(edges, objectSize);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/objectness.png").string(), objectness, false, {0, 0}, true);
  const auto objects = CalculateObjects(objectness, objectnessThreshold, minObjectSize);
  Saveimg((GetProjectDirectoryPath() / "data/debug/ObjectDetection/objects.png").string(), DrawObjects(source, objects));
}
