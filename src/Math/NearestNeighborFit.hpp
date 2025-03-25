#pragma once

inline int FindNearestNeighborIndex(const std::vector<cv::Point2f>& pts, cv::Point2f pt)
{
  int idx = 0;
  double mindist = std::numeric_limits<double>::max();
  double dist;
  for (size_t i = 0; i < pts.size(); i++)
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

inline cv::Mat NearestNeighborFit(const std::vector<cv::Point2f>& pts, const std::vector<double>& zdata, double xmin, double xmax, double ymin, double ymax, int xcnt, int ycnt)
{
  cv::Mat out = cv::Mat::zeros(ycnt, xcnt, CV_32F);
  for (int r = 0; r < out.rows; r++)
  {
    for (int c = 0; c < out.cols; c++)
    {
      double x = xmin + ((float)c / (out.cols - 1)) * (xmax - xmin);
      double y = ymin + ((float)r / (out.rows - 1)) * (ymax - ymin);
      cv::Point2f pt(x, y);
      out.at<float>(r, c) = zdata[FindNearestNeighborIndex(pts, pt)];
    }
  }
  return out;
}

inline void UpdateHighest(std::vector<std::pair<int, double>>& proxidxs, int idx, double val)
{
  int maxidx = 0;
  double maxval = 0;
  for (size_t i = 0; i < proxidxs.size(); i++)
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

inline cv::Mat WeightedNearestNeighborFit(const std::vector<cv::Point2f>& pts, const std::vector<double>& zdata, double xmin, double xmax, double ymin, double ymax, int xcnt,
    int ycnt, int proxpts = 10, double proxcoeff = 7)
{
  cv::Mat out = cv::Mat::zeros(ycnt, xcnt, CV_32F);
  proxpts = std::min(proxpts, (int)pts.size());
  for (int r = 0; r < out.rows; r++)
  {
    for (int c = 0; c < out.cols; c++)
    {
      // get correspoinding point values
      double x = xmin + ((float)c / (out.cols - 1)) * (xmax - xmin);
      double y = ymin + ((float)r / (out.rows - 1)) * (ymax - ymin);
      cv::Point2f pt(x, y);

      // get first proxpts point indices, maxdistance
      double distance = 0;
      double maxdistance = 0;
      std::vector<std::pair<int, double>> proxidxs(proxpts);
      for (size_t i = 0; i < pts.size(); i++)
      {
        distance = Magnitude(pts[i] - pt);
        if (distance > maxdistance)
          maxdistance = distance;

        if (i < static_cast<size_t>(proxpts))
          proxidxs.push_back(std::make_pair(i, distance));
        else
          UpdateHighest(proxidxs, i, distance);
      }

      // weighted average
      double weight = 0;
      double weightsum = 0;
      for (auto i : proxidxs)
      {
        weight = std::max(1. - Magnitude(pts[i.first] - pt) / (maxdistance / proxcoeff), 0.);
        weightsum += weight;
        out.at<float>(r, c) += weight * zdata[i.first];
      }
      out.at<float>(r, c) /= weightsum;
    }
  }
  return out;
}
