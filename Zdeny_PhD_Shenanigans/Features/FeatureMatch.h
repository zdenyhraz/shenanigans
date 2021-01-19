#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fit/polyfit.h"
#include "Fit/nnfit.h"
#include "Draw/combinepics.h"
#include "Utils/export.h"

static constexpr int piccnt = 10;               // number of pics
static constexpr double scale = 10;             // scale for visualization
static constexpr double kmpp = 696010. / 378.3; // kilometers per pixel
static constexpr double dt = 11.88;             // dt temporally adjacent pics

static constexpr double arrow_scale = 8;               // size of the arrow
static constexpr double arrow_thickness = scale / 1.0; // arrow thickness
static constexpr double text_scale = scale / 2;        // text scale
static constexpr double text_thickness = scale / 1.5;  // text thickness

enum FeatureType
{
  SURF,
  BRISK,
  ORB,
  MSER,
  FAST,
  AGAST,
  GFTT,
  KAZE,
  AKAZE
};

struct FeatureMatchData
{
  FeatureType ftype;
  double thresh;
  int matchcnt;
  double quanB;
  double quanT;
  std::string path1;
  std::string path2;
  std::string path;
  std::string pathout;
  int degree;
  int proxpts;
  double proxcoeff;
  double overlapdistance;
  bool drawOverlapCircles = false;
  double ratioThreshold = 0.7;
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
  case FeatureType::BRISK:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  case FeatureType::ORB:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  case FeatureType::MSER:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  case FeatureType::FAST:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  case FeatureType::AGAST:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  case FeatureType::GFTT:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  case FeatureType::KAZE:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  case FeatureType::AKAZE:
    return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
  }
  return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
}

inline Ptr<Feature2D> GetFeatureDetector(const FeatureMatchData& data)
{
  switch (data.ftype)
  {
  case FeatureType::SURF:
    return xfeatures2d::SURF::create(std::min(data.thresh, 500.));
  case FeatureType::BRISK:
    return BRISK::create();
  case FeatureType::ORB:
    return ORB::create();
  case FeatureType::MSER:
    return MSER::create();
  case FeatureType::FAST:
    return ORB::create();
  case FeatureType::AGAST:
    return ORB::create();
  case FeatureType::GFTT:
    return ORB::create();
  case FeatureType::KAZE:
    return KAZE::create();
  case FeatureType::AKAZE:
    return AKAZE::create();
  }
  return ORB::create();
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
  resize(out, out, Size(scale * out.cols, scale * out.rows), 0, 0, INTER_LANCZOS4);

  static constexpr double kMinSpeed = 300;
  static constexpr double kMaxSpeed = 1000;
  double minspd = std::max(getQuantile(speeds_all, data.quanB), kMinSpeed);
  double maxspd = std::min(getQuantile(speeds_all, data.quanT), kMaxSpeed);

  for (auto it = matches_all.rbegin(); it != matches_all.rend(); ++it)
  {
    const auto& [idx, pic, match, overlap] = *it;

    if (overlap)
      continue;

    const auto shift = GetFeatureMatchShift(match, kp1_all[pic], kp2_all[pic]);
    const double spd = magnitude(shift) * kmpp / dt;
    const double dir = toDegrees(atan2(-shift.y, shift.x));

    if (spd < minspd || spd > maxspd)
      continue;

    if (dir < -180 || dir > -90)
      continue;

    auto pts = GetFeatureMatchPoints(match, kp1_all[pic], kp2_all[pic]);
    Point2f arrStart = scale * pts.first;
    Point2f arrEnd = scale * pts.first + arrow_scale * (scale * pts.second - scale * pts.first);
    Point2f textpos = (arrStart + arrEnd) / 2;
    Scalar color = colorMapJet(spd, minspd * 0.5, maxspd * 1.2);
    textpos.x -= scale * 1;
    textpos.y += scale * 7;
    arrowedLine(out, arrStart, arrEnd, color, arrow_thickness, LINE_AA, 0, 0.15);
    putText(out, to_stringp(spd, 1), textpos, 1, text_scale, color, text_thickness, LINE_AA);

    if (data.drawOverlapCircles)
      circle(out, arrStart, scale * data.overlapdistance, Scalar(0, 255, 255), text_thickness, LINE_AA);
  }

  return out;
}

inline void featureMatch(const FeatureMatchData& data)
{
  LOG_FUNCTION("FeatureMatch");

  Mat img_base = imread(data.path + "5.PNG", IMREAD_GRAYSCALE);
  std::vector<std::vector<DMatch>> matches_all(piccnt - 1);
  std::vector<std::vector<KeyPoint>> keypoints1_all(piccnt - 1);
  std::vector<std::vector<KeyPoint>> keypoints2_all(piccnt - 1);
  std::vector<std::vector<double>> speeds_all(piccnt - 1);

#pragma omp parallel for
  for (int pic = 0; pic < piccnt - 1; pic++)
  {
    std::string path1 = data.path + to_string(pic) + ".PNG";
    std::string path2 = data.path + to_string(pic + 1) + ".PNG";
    LOG_INFO("Matching images {} - {} ({} - {})", pic, pic + 1, path1, path2);
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
}
