#pragma once

struct Object
{
  enum class Status
  {
    Valid,
    BadArea,
    BadElongatedness
  };

  cv::Point center;
  std::vector<cv::Point> contour;
  Status status = Status::Valid;
};

inline cv::Mat CalculateObjectness(const cv::Mat& edges, i32 objectSize)
{
  LOG_FUNCTION;
  cv::Mat edgesNorm(edges.size(), edges.type());
  cv::normalize(edges, edgesNorm, 0, 1, cv::NORM_MINMAX);
  cv::Mat objectness(edgesNorm.size(), edgesNorm.type());
  const auto blurSize = GetNearestOdd(1.7 * objectSize);
  cv::GaussianBlur(edgesNorm, objectness, cv::Size(blurSize, blurSize), 0, 0, cv::BORDER_REPLICATE);
  return objectness;
}

inline std::vector<Object> CalculateObjects(const cv::Mat& objectness, f32 minObjectArea, f32 maxObjectElongatedness)
{
  LOG_FUNCTION;
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(objectness, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
  std::vector<Object> objects;
  for (const auto& contour : contours)
  {
    const auto rect = cv::minAreaRect(contour);
    const f32 width = std::min(rect.size.width, rect.size.height);
    const f32 height = std::max(rect.size.width, rect.size.height);
    const f32 elongatedness = height / width;
    const f32 area = width * height;
    using enum Object::Status;
    Object::Status status = Valid;

    if (elongatedness > maxObjectElongatedness) // filter out "needle" artifacts
      status = BadElongatedness;
    else if (area < minObjectArea) // filter out small objects
      status = BadArea;
    else
      status = Valid;

    const auto center = std::accumulate(contour.begin(), contour.end(), cv::Point{0, 0}) / static_cast<i32>(contour.size());
    objects.emplace_back(center, contour, status);
  }
  return objects;
}

inline cv::Mat DrawObjects(const cv::Mat& source, const std::vector<Object>& objects)
{
  LOG_FUNCTION;
  cv::Mat src = source.clone();
  cv::normalize(src, src, 0, 255, cv::NORM_MINMAX);
  src.convertTo(src, CV_8U);
  cv::Mat out;
  cv::applyColorMap(src, out, cv::COLORMAP_VIRIDIS);

  const auto colorValid = cv::Scalar(0, 0, 255);
  const auto colorBadArea = cv::Scalar(255, 0, 0);
  const auto colorBadElongatedness = cv::Scalar(0, 165, 255);
  const auto thickness = std::clamp(0.005 * out.rows, 1., 100.);
  using enum Object::Status;
  for (const auto& object : objects)
  {
    switch (object.status)
    {
    case Valid:
      cv::drawContours(out, std::vector<std::vector<cv::Point>>{object.contour}, -1, colorValid, thickness, cv::LINE_AA);
      break;
    case BadArea:
      cv::drawContours(out, std::vector<std::vector<cv::Point>>{object.contour}, -1, colorBadArea, thickness, cv::LINE_AA);
      break;
    case BadElongatedness:
      cv::drawContours(out, std::vector<std::vector<cv::Point>>{object.contour}, -1, colorBadElongatedness, thickness, cv::LINE_AA);
      break;
    }
  }
  return out;
}

inline cv::Mat CalculateEdges(const cv::Mat& source, i32 edgeSize)
{
  LOG_FUNCTION;
  cv::Mat edgesX, edgesY;
  cv::Sobel(source, edgesX, CV_32F, 1, 0, std::clamp(edgeSize, 3, 31), 1, 0, cv::BORDER_REPLICATE);
  cv::Sobel(source, edgesY, CV_32F, 0, 1, std::clamp(edgeSize, 3, 31), 1, 0, cv::BORDER_REPLICATE);
  edgesX = cv::abs(edgesX);
  edgesY = cv::abs(edgesY);
  cv::normalize(edgesX, edgesX, 0, 1, cv::NORM_MINMAX);
  cv::normalize(edgesY, edgesY, 0, 1, cv::NORM_MINMAX);
  cv::Mat edges(edgesX.size(), CV_32F);
  for (i32 r = 0; r < edges.rows; ++r)
  {
    auto edgesp = edges.ptr<f32>(r);
    auto edgesXp = edgesX.ptr<f32>(r);
    auto edgesYp = edgesY.ptr<f32>(r);
    for (i32 c = 0; c < edges.cols; ++c)
      edgesp[c] = std::max(edgesXp[c], edgesYp[c]);
  }
  return edges;
}

struct SobelObjectnessParameters
{
  f32 blurSize = 0.015;
  f32 edgeSize = 0.0025;
  f32 edgeThreshold = 0.16;
  f32 objectSize = 0.02;
  f32 objectnessThreshold = 0.25;
  f32 minObjectArea = 0.001;
  f32 maxObjectElongatedness = 8;
};

inline std::vector<Object> DetectObjectsSobelObjectness(const cv::Mat& source, const SobelObjectnessParameters& params)
{
  LOG_FUNCTION;
  Plot::Plot("source", source);

  // blur for more robust edge detection
  cv::Mat blurred(source.size(), source.type());
  const auto blurSize = GetNearestOdd(params.blurSize * source.rows);
  cv::GaussianBlur(source, blurred, cv::Size(blurSize, blurSize), 0);
  Plot::Plot("blurred", blurred);

  // calclate absolute Sobel x/y edges
  cv::Mat edges = CalculateEdges(blurred, GetNearestOdd(params.edgeSize * source.rows));
  Plot::Plot("edges", edges);

  // threshold the edges
  if (params.edgeThreshold > 0)
    cv::threshold(edges, edges, params.edgeThreshold, 1, cv::THRESH_BINARY);
  Plot::Plot("edges_thr", edges);

  // calculate objectness (local "edginess")
  cv::Mat objectness = CalculateObjectness(edges, params.objectSize * source.rows);
  Plot::Plot("objectness", objectness);

  // threshold objectness
  cv::threshold(objectness, objectness, params.objectnessThreshold, 255, cv::THRESH_BINARY);
  objectness.convertTo(objectness, CV_8U);
  Plot::Plot("objectness_thr", objectness);

  // calclate objects (find thresholded objectness contours)
  const auto objects = CalculateObjects(objectness, params.minObjectArea * source.rows * source.rows, params.maxObjectElongatedness);
  Plot::Plot("objects", DrawObjects(source, objects));

  return objects;
}
