#pragma once

inline i32 FindNearestNeighborIndex(const std::vector<cv::Point2f>& pts, cv::Point2f pt)
{
  i32 idx = 0;
  f64 mindist = std::numeric_limits<f64>::max();
  f64 dist;
  for (usize i = 0; i < pts.size(); i++)
  {
    dist = Magnitude(pts[i] - pt);
    if (dist < mindist)
    {
      mindist = dist;
      idx = i;
    }
  }
  return idx;
}

inline cv::Mat NearestNeighborFit(const std::vector<cv::Point2f>& pts, const std::vector<f64>& zdata, f64 xmin, f64 xmax, f64 ymin, f64 ymax, i32 xcnt, i32 ycnt)
{
  cv::Mat out = cv::Mat::zeros(ycnt, xcnt, CV_32F);
  for (i32 r = 0; r < out.rows; r++)
  {
    for (i32 c = 0; c < out.cols; c++)
    {
      f64 x = xmin + ((f32)c / (out.cols - 1)) * (xmax - xmin);
      f64 y = ymin + ((f32)r / (out.rows - 1)) * (ymax - ymin);
      cv::Point2f pt(x, y);
      out.at<f32>(r, c) = zdata[FindNearestNeighborIndex(pts, pt)];
    }
  }
  return out;
}

inline void UpdateHighest(std::vector<std::pair<i32, f64>>& proxidxs, i32 idx, f64 val)
{
  i32 maxidx = 0;
  f64 maxval = 0;
  for (usize i = 0; i < proxidxs.size(); i++)
  {
    if (proxidxs[i].second > maxval)
    {
      maxval = proxidxs[i].second;
      maxidx = i;
    }
  }

  if (proxidxs[maxidx].second > val)
  {
    proxidxs[maxidx] = std::make_pair(idx, val);
  }
}

inline cv::Mat WeightedNearestNeighborFit(
    const std::vector<cv::Point2f>& pts, const std::vector<f64>& zdata, f64 xmin, f64 xmax, f64 ymin, f64 ymax, i32 xcnt, i32 ycnt, i32 proxpts = 10, f64 proxcoeff = 7)
{
  cv::Mat out = cv::Mat::zeros(ycnt, xcnt, CV_32F);
  proxpts = std::min(proxpts, (i32)pts.size());
  for (i32 r = 0; r < out.rows; r++)
  {
    for (i32 c = 0; c < out.cols; c++)
    {
      // get correspoinding point values
      f64 x = xmin + ((f32)c / (out.cols - 1)) * (xmax - xmin);
      f64 y = ymin + ((f32)r / (out.rows - 1)) * (ymax - ymin);
      cv::Point2f pt(x, y);

      // get first proxpts point indices, maxdistance
      f64 distance = 0;
      f64 maxdistance = 0;
      std::vector<std::pair<i32, f64>> proxidxs(proxpts);
      for (usize i = 0; i < pts.size(); i++)
      {
        distance = Magnitude(pts[i] - pt);
        if (distance > maxdistance)
          maxdistance = distance;

        if (i < static_cast<usize>(proxpts))
          proxidxs.push_back(std::make_pair(i, distance));
        else
          UpdateHighest(proxidxs, i, distance);
      }

      // weighted average
      f64 weight = 0;
      f64 weightsum = 0;
      for (auto i : proxidxs)
      {
        weight = std::max(1. - Magnitude(pts[i.first] - pt) / (maxdistance / proxcoeff), 0.);
        weightsum += weight;
        out.at<f32>(r, c) += weight * zdata[i.first];
      }
      out.at<f32>(r, c) /= weightsum;
    }
  }
  return out;
}
