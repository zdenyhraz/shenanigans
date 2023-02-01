#pragma once

struct Object
{
  cv::Point center;
  std::vector<cv::Point> contour;
  bool filtered = false;
};

inline cv::Mat CalculateObjectness(const cv::Mat& edges, i32 objectSize, bool gaussianObjectness)
{
  LOG_FUNCTION;
  cv::Mat edgesNorm(edges.size(), edges.type());
  cv::normalize(edges, edgesNorm, 0, 1, cv::NORM_MINMAX);
  cv::Mat objectness(edgesNorm.size(), edgesNorm.type());

  if (gaussianObjectness)
  {
    const auto blurSize = GetNearestOdd(1.7 * objectSize);
    cv::GaussianBlur(edgesNorm, objectness, cv::Size(blurSize, blurSize), 0);
  }
  else
  {
    cv::Mat window = Kirkl<f32>(objectSize);
    std::atomic<usize> progress = 0;
    const auto objectnessMultiplier = 1.0f / (objectSize * objectSize);
    LOG_PROGRESS_NAME("CalculateObjectness");
#pragma omp parallel for
    for (i32 r = objectSize / 2; r < edgesNorm.rows - objectSize / 2; ++r)
    {
      LOG_PROGRESS(static_cast<f32>(++progress) / (edgesNorm.rows - objectSize));
      for (i32 c = objectSize / 2; c < edgesNorm.cols - objectSize / 2; ++c)
        objectness.at<f32>(r, c) = objectnessMultiplier * cv::sum(RoiCropRef(edgesNorm, c, r, objectSize, objectSize).mul(window))[0];
    }
    LOG_PROGRESS_RESET;
  }
  return objectness; // pixel average
}

inline std::vector<Object> CalculateObjects(const cv::Mat& objectness, f32 minObjectSize)
{
  LOG_FUNCTION;
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(objectness, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
  LOG_DEBUG("Found {} objects", contours.size());

  std::vector<Object> objects;
  usize filteredCount = 0;
  for (const auto& contour : contours)
  {
    const auto minRect = cv::minAreaRect(contour);
    bool filtered = false;

    // filter out small objects
    if (minRect.size.width < minObjectSize and minRect.size.height < minObjectSize)
      filtered = true;

    // filter out "needle" artifacts
    if (minRect.size.width < minObjectSize or minRect.size.height < minObjectSize)
      filtered = true;

    if (filtered)
      ++filteredCount;

    const auto center = std::accumulate(contour.begin(), contour.end(), cv::Point{0, 0}) / static_cast<i32>(contour.size());
    objects.emplace_back(center, contour, filtered);
  }
  LOG_DEBUG("Filtered {} objects", filteredCount);
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

  const auto color = cv::Scalar(0, 0, 255);
  const auto colorFiltered = cv::Scalar(255, 0, 0);
  const auto thickness = std::clamp(0.003 * out.rows, 1., 100.);
  for (const auto& object : objects)
    cv::drawContours(out, std::vector<std::vector<cv::Point>>{object.contour}, -1, object.filtered ? colorFiltered : color, thickness, cv::LINE_AA);

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
  f32 blurSizeMultiplier = 0.015;
  f32 edgeSizeMultiplier = 0.0025;
  f32 edgeThreshold = 0.16;
  bool gaussianObjectness = true;
  f32 objectSizeMultiplier = 0.02;
  f32 objectnessThreshold = 0.2;
  f32 minObjectSizeMultiplier = 0.02;
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
  if (params.edgeThreshold > 0)
    cv::threshold(edges, edges, params.edgeThreshold, 1, cv::THRESH_BINARY);
  Plot::Plot("edges_thr", edges);

  // calculate objectness (local "edginess")
  cv::Mat objectness = CalculateObjectness(edges, params.objectSizeMultiplier * source.rows, params.gaussianObjectness);
  Plot::Plot("objectness", objectness);

  // threshold objectness
  cv::threshold(objectness, objectness, params.objectnessThreshold, 255, cv::THRESH_BINARY);
  objectness.convertTo(objectness, CV_8U);
  Plot::Plot("objectness_thr", objectness);

  // calclate objects (find thresholded objectness contours)
  const auto objects = CalculateObjects(objectness, params.minObjectSizeMultiplier * source.rows);
  Plot::Plot("objects", DrawObjects(source, objects));

  return objects;
}
