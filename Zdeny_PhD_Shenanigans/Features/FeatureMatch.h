#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fit/polyfit.h"
#include "Fit/nnfit.h"
#include "Draw/combinepics.h"
#include "Utils/export.h"

static constexpr int piccnt = 8;                // number of pics
static constexpr double kmpp = 696010. / 378.3; // kilometers per pixel
static constexpr double dt = 11.8;              // dt temporally adjacent pics

static constexpr double arrow_scale = 12;
static constexpr double arrow_thickness = 0.0015;
static constexpr double text_scale = 0.001;
static constexpr double text_thickness = arrow_thickness * 1.0;
static constexpr double text_xoffset = -0.0025;
static constexpr double text_yoffset = -7 * text_xoffset;

enum FeatureType
{
  SURF,
  SIFT,
};

struct FeatureMatchData
{
  FeatureType ftype;
  double thresh;
  int matchcnt;
  double minSpeed;
  double maxSpeed;
  std::string path;
  std::string pathout;
  double overlapdistance;
  bool drawOverlapCircles = false;
  double ratioThreshold = 0.7;
  double upscale = 1;
  bool surfExtended = false;
  bool surfUpright = false;
  int nOctaves = 4;
  int nOctaveLayers = 3;
};

inline Point2f GetFeatureMatchShift(const DMatch& match, const std::vector<KeyPoint>& kp1, const std::vector<KeyPoint>& kp2)
{
  return kp2[match.trainIdx].pt - kp1[match.queryIdx].pt;
}

inline std::pair<Point2f, Point2f> GetFeatureMatchPoints(const DMatch& match, const std::vector<KeyPoint>& kp1, const std::vector<KeyPoint>& kp2)
{
  return std::make_pair(kp1[match.queryIdx].pt, kp2[match.trainIdx].pt);
}

inline int GetFeatureTypeMatcher(const FeatureMatchData& data)
{
  switch (data.ftype)
  {
  case FeatureType::SURF:
    return DescriptorMatcher::MatcherType::BRUTEFORCE;
  case FeatureType::SIFT:
    return DescriptorMatcher::MatcherType::BRUTEFORCE;
  }

  throw std::runtime_error("Unknown feature type");
}

inline Ptr<Feature2D> GetFeatureDetector(const FeatureMatchData& data)
{
  using namespace xfeatures2d;
  switch (data.ftype)
  {
  case FeatureType::SURF:
    return SURF::create(std::min(data.thresh, 500.), std::max(data.nOctaves, 1), std::max(data.nOctaveLayers, 1), data.surfExtended, data.surfUpright);
  case FeatureType::SIFT:
    return SIFT::create(0, std::max(data.nOctaveLayers, 1), 0, 1e5);
  }

  throw std::runtime_error("Unknown feature type");
}

inline void exportFeaturesToCsv(const std::string& path, const std::vector<Point2f>& points, const std::vector<double>& speeds, const std::vector<double>& directions)
{
  std::string pth = path + "features.csv";
  std::ofstream csv(pth, std::ios::out | std::ios::trunc);
  csv << "X,Y,SPD,DIR" << endl;
  for (int i = 0; i < points.size(); i++)
  {
    csv << points[i].x << "," << points[i].y << "," << speeds[i] << "," << directions[i] << endl;
  }
  LOG_INFO("Feature data exported to {}", pth);
}

inline Mat DrawFeatureMatchArrows(const Mat& img, const std::vector<std::tuple<size_t, size_t, DMatch, bool>>& matches_all, const std::vector<std::vector<KeyPoint>>& kp1_all,
                                  const std::vector<std::vector<KeyPoint>>& kp2_all, const std::vector<std::vector<double>>& speeds_all, const FeatureMatchData& data)
{
  LOG_FUNCTION("DrawFeatureMatchArrows");
  Mat out;
  cvtColor(img, out, COLOR_GRAY2BGR);

  if (data.upscale != 1)
    resize(out, out, Size(data.upscale * out.cols, data.upscale * out.rows), 0, 0, INTER_LANCZOS4);

  double minspd = std::max(getQuantile(speeds_all, 0), data.minSpeed);
  double maxspd = std::min(getQuantile(speeds_all, 1), data.maxSpeed);
  size_t drawcounter = 0;

  for (auto it = matches_all.rbegin(); it != matches_all.rend(); ++it)
  {
    const auto& [idx, pic, match, overlap] = *it;

    if (overlap)
      continue;

    if (img.at<uchar>(kp1_all[pic][match.queryIdx].pt) < 10)
    {
      LOG_TRACE("Skipping match {}: black region", idx);
      continue;
    }

    const auto shift = GetFeatureMatchShift(match, kp1_all[pic], kp2_all[pic]);
    const double spd = magnitude(shift) * kmpp / dt;
    const double dir = toDegrees(atan2(-shift.y, shift.x));

    if (spd < data.minSpeed)
    {
      LOG_TRACE("Skipping match {}: speed {} km/s too slow", idx, spd);
      continue;
    }

    if (spd > data.maxSpeed)
    {
      LOG_WARNING("Skipping match {}: speed {} km/s too fast", idx, spd);
      continue;
    }

    if (dir < -170 || dir > -100)
    {
      LOG_TRACE("Skipping match {}: direction {} deg off limits", idx, dir);
      continue;
    }

    auto pts = GetFeatureMatchPoints(match, kp1_all[pic], kp2_all[pic]);
    Point2f arrStart = data.upscale * pts.first;
    Point2f arrEnd = data.upscale * pts.first + arrow_scale * out.cols / maxspd * (pts.second - pts.first);
    Point2f textpos = (arrStart + arrEnd) / 2;
    Scalar color = colorMapJet(spd, minspd * 0.5, maxspd * 1.2);
    textpos.x += text_xoffset * out.cols;
    textpos.y += text_yoffset * out.cols;
    arrowedLine(out, arrStart, arrEnd, color, arrow_thickness * out.cols, LINE_AA, 0, 0.1);
    putText(out, fmt::format("{:.0f}({})", spd, drawcounter), textpos, 1, text_scale * out.cols, color, text_thickness * out.cols, LINE_AA);

    if (data.drawOverlapCircles)
      circle(out, arrStart, data.upscale * data.overlapdistance, Scalar(0, 255, 255), text_thickness * out.cols, LINE_AA);

    drawcounter++;
  }

  LOG_INFO("Drew {} out of {} matches", drawcounter, matches_all.size());

  return out;
}

inline void featureMatch(const FeatureMatchData& data)
try
{
  LOG_FUNCTION("FeatureMatch");

  Mat img_base = imread(data.path + "5.PNG", IMREAD_GRAYSCALE);
  std::vector<std::vector<DMatch>> matches_all(piccnt - 1);
  std::vector<std::vector<KeyPoint>> keypoints1_all(piccnt - 1);
  std::vector<std::vector<KeyPoint>> keypoints2_all(piccnt - 1);
  std::vector<std::vector<double>> speeds_all(piccnt - 1);

#pragma omp parallel for
  for (int pic = 1; pic < piccnt - 1; pic++)
  {
    std::string path1 = data.path + to_string(pic) + ".PNG";
    std::string path2 = data.path + to_string(pic + 1) + ".PNG";
    LOG_DEBUG(fmt::format("Matching images {} & {}", path1, path2));
    Mat img1 = imread(path1, IMREAD_GRAYSCALE);
    Mat img2 = imread(path2, IMREAD_GRAYSCALE);

    // detect the keypoints, compute the descriptors
    Ptr<Feature2D> detector = GetFeatureDetector(data);
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(img2, noArray(), keypoints2, descriptors2);
    keypoints1_all[pic] = keypoints1;
    keypoints2_all[pic] = keypoints2;

    // matching descriptor vectors
    auto matcher = DescriptorMatcher::create((DescriptorMatcher::MatcherType)GetFeatureTypeMatcher(data));
    std::vector<std::vector<DMatch>> knn_matches;
    matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

    // filter matches using the Lowe's ratio test
    std::vector<DMatch> matches;
    for (size_t i = 0; i < knn_matches.size(); i++)
      if (knn_matches[i][0].distance < data.ratioThreshold * knn_matches[i][1].distance)
        matches.push_back(knn_matches[i][0]);

    // get first best fits
    std::sort(matches.begin(), matches.end(), [&](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });
    matches = std::vector<DMatch>(matches.begin(), matches.begin() + min(data.matchcnt, (int)matches.size()));
    matches_all[pic] = matches;

    // calculate feature shifts
    std::vector<Point2f> shifts(matches.size());
    std::vector<double> speeds(matches.size());
    for (int i = 0; i < matches.size(); i++)
    {
      shifts[i] = GetFeatureMatchShift(matches[i], keypoints1, keypoints2);
      speeds[i] = magnitude(shifts[i]) * kmpp / dt;
    }
    speeds_all[pic] = speeds;

    if (0)
    {
      LOG_FUNCTION("Draw matches");
      Mat img_matches;
      drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar(0, 255, 255), Scalar(0, 255, 0), std::vector<char>(), DrawMatchesFlags::DEFAULT);
      showimg(img_matches, "Good matches");
    }
  }

  std::vector<std::tuple<size_t, size_t, DMatch, bool>> matches_all_serialized;
  if (1)
  {
    LOG_FUNCTION("Sort matches from fastest to slowest s-wind speeds");

    size_t idx = 0;
    for (size_t pic = 0; pic < piccnt - 1; pic++)
      for (const auto& match : matches_all[pic])
        matches_all_serialized.push_back({idx++, pic, match, false});

    std::sort(matches_all_serialized.begin(), matches_all_serialized.end(), [&](const auto& a, const auto& b) {
      const auto& [idx1, pic1, match1, ignore1] = a;
      const auto& [idx2, pic2, match2, ignore2] = b;
      const auto shift1 = GetFeatureMatchShift(match1, keypoints1_all[pic1], keypoints2_all[pic1]);
      const auto shift2 = GetFeatureMatchShift(match2, keypoints1_all[pic2], keypoints2_all[pic2]);
      const auto spd1 = magnitude(shift1) * kmpp / dt;
      const auto spd2 = magnitude(shift2) * kmpp / dt;
      return spd1 > spd2;
    });

    size_t newidx = 0;
    for (auto& [idx, pic, match, overlap] : matches_all_serialized)
      idx = newidx++;
  }

  if (data.overlapdistance > 0)
  {
    LOG_FUNCTION("Filter overlapping matches");

    for (const auto& [idx, pic, match, overlap] : matches_all_serialized)
    {
      // not filtering already filtered matches
      if (overlap)
        continue;

      for (auto& [otheridx, otherpic, othermatch, otheroverlap] : matches_all_serialized)
      {
        // not filtering already filtered matches
        if (otheroverlap)
          continue;

        // not filtering faster matches
        if (otheridx <= idx)
          continue;

        const auto shift = keypoints1_all[pic][match.queryIdx].pt - keypoints1_all[otherpic][othermatch.queryIdx].pt;
        const double distance = magnitude(shift);
        otheroverlap = distance < data.overlapdistance;
      }
    }
  }

  const auto arrows = DrawFeatureMatchArrows(img_base, matches_all_serialized, keypoints1_all, keypoints2_all, speeds_all, data);
  showimg(arrows, "Match arrows", false, 0, 1, 1200);
  saveimg("../articles/swind/arrows/arrows.png", arrows);
}
catch (const std::exception& e)
{
  LOG_ERROR("Feature match error: {}", e.what());
}
