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

inline std::vector<Object> CalculateObjects(const cv::Mat& objectness, f32 minObjectSize)
{
  LOG_FUNCTION;
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(objectness, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
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

inline cv::Mat CalculateEdges(const cv::Mat& source, i32 edgeSize)
{
  cv::Mat edgesX, edgesY;
  cv::Sobel(source, edgesX, CV_32F, 1, 0, edgeSize, 1, 0, cv::BORDER_REPLICATE);
  cv::Sobel(source, edgesY, CV_32F, 0, 1, edgeSize, 1, 0, cv::BORDER_REPLICATE);
  edgesX = cv::abs(edgesX);
  edgesY = cv::abs(edgesY);
  cv::normalize(edgesX, edgesX, 0, 1, cv::NORM_MINMAX);
  cv::normalize(edgesY, edgesY, 0, 1, cv::NORM_MINMAX);
  cv::Mat edges = edgesX + edgesY;
  cv::normalize(edges, edges, 0, 1, cv::NORM_MINMAX);
  return edges;
}

inline i32 GetNearestOdd(i32 value)
{
  return value % 2 == 0 ? value + 1 : value;
}

struct SobelObjectnessParameters
{
  f32 objectSizeMultiplier = 0.05;    // 0.02
  f32 blurSizeMultiplier = 0.05;      // 0.005
  f32 edgeSizeMultiplier = 0.0025;    // 0.0025
  f32 edgeThreshold = 0.7;            // 0.3 relative normalized
  f32 objectnessThreshold = 0.01;     // 0.01
  f32 minObjectSizeMultiplier = 0.02; // 0.02
};

inline std::vector<Object> DetectObjectsSobelObjectness(const cv::Mat& source, const SobelObjectnessParameters& params)
{
  LOG_FUNCTION;
  Plot::Plot("source", source);

  // blur for more robust edge detection
  cv::Mat blurred(source.size(), source.type());
  const auto blurSize = GetNearestOdd(params.blurSizeMultiplier * source.rows);
  cv::GaussianBlur(source, blurred, cv::Size(blurSize, blurSize), 0);
  Plot::Plot("blurred", blurred);

  // calclate absolute Sobel x/y edges
  cv::Mat edges = CalculateEdges(blurred, GetNearestOdd(params.edgeSizeMultiplier * source.rows));
  Plot::Plot("edges", edges);

  // threshold the edges
  cv::threshold(edges, edges, params.edgeThreshold, 1, cv::THRESH_BINARY);
  Plot::Plot("edges_thr", edges);

  // calculate objectness (local "edginess")
  cv::Mat objectness = CalculateObjectness(edges, params.objectSizeMultiplier * source.rows);
  Plot::Plot("objectness", objectness);

  // threshold objectness
  cv::threshold(objectness, objectness, params.objectnessThreshold, 255, cv::THRESH_BINARY);
  objectness.convertTo(objectness, CV_8U);
  Plot::Plot("objectness_thr", objectness);

  // calclate objects (find thresholded objectness contours)
  const auto objects = CalculateObjects(objectness, params.minObjectSizeMultiplier * source.rows);
  CvPlot::Plot({.name = "objects", .z = DrawObjects(source, objects), .savepath = (GetProjectDirectoryPath() / "data/debug/ObjectDetection/objects.png").string()});

  return objects;
}
