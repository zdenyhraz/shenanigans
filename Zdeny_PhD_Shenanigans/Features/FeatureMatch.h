#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fit/polyfit.h"
#include "Fit/nnfit.h"
#include "Draw/combinepics.h"
#include "Utils/export.h"

static constexpr int piccnt = 9;                // number of pics
static constexpr double kmpp = 696010. / 378.3; // kilometers per pixel
static constexpr double dt = 11.8;              // dt seconds temporally adjacent pics
static constexpr double arrow_scale = 12;
static constexpr double arrow_thickness = 0.0015;
static constexpr double text_scale = 0.001;
static constexpr double text_thickness = arrow_thickness * 1.0;
static constexpr double text_xoffset = 0.0075;
static constexpr double text_yoffset = 2 * text_xoffset;

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
  std::string path1;
  std::string path2;
  double overlapdistance;
  bool drawOverlapCircles = false;
  double ratioThreshold = 0.7;
  double upscale = 1;
  bool surfExtended = false;
  bool surfUpright = false;
  int nOctaves = 4;
  int nOctaveLayers = 3;
  bool mask = true;
};

inline Point2f GetFeatureMatchShift(const DMatch& match, const std::vector<KeyPoint>& kp1, const std::vector<KeyPoint>& kp2)
{
  return kp2[match.trainIdx].pt - kp1[match.queryIdx].pt;
}

inline std::pair<Point2f, Point2f> GetFeatureMatchPoints(const DMatch& match, const std::vector<KeyPoint>& kp1, const std::vector<KeyPoint>& kp2)
{
  return std::make_pair(kp1[match.queryIdx].pt, kp2[match.trainIdx].pt);
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

inline void ExportFeaturesToCsv(const std::string& path, const std::vector<Point2f>& points, const std::vector<double>& speeds, const std::vector<double>& directions)
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
    const std::vector<std::vector<KeyPoint>>& kp2_all, const FeatureMatchData& data, bool drawSpeed)
{
  LOG_FUNCTION("DrawFeatureMatchArrows");
  Mat out;
  cvtColor(img, out, COLOR_GRAY2BGR);

  if (data.upscale != 1)
    resize(out, out, Size(data.upscale * out.cols, data.upscale * out.rows), 0, 0, INTER_LINEAR);

  double minspd = std::numeric_limits<double>::max();
  double maxspd = std::numeric_limits<double>::min();
  std::vector<bool> shouldDraw(matches_all.size(), false);
  std::vector<double> removeSpeeds = {}; // 639, 652

  if (false)
    for (int r = 0; r < out.rows; ++r)
      for (int c = 0; c < out.cols; ++c)
        if (((float)c / img.cols + (float)r / img.rows) / 2 < 0.5)
          out.at<Vec3b>(r, c) = (out.at<Vec3b>(r, c) + Vec3b(0, 0, 255)) / 2;

  for (auto it = matches_all.rbegin(); it != matches_all.rend(); ++it)
  {
    const auto& [idx, pic, match, overlap] = *it;

    if (overlap)
      continue;

    const auto& point = kp1_all[pic][match.queryIdx].pt;

    if (data.mask && (((float)point.x / img.cols + (float)point.y / img.rows) / 2 < 0.5))
    {
      LOG_TRACE("Skipping match {}: masked out", idx);
      continue;
    }

    if (img.at<uchar>(point) < 10)
    {
      LOG_TRACE("Skipping match {}: black region", idx);
      continue;
    }

    const auto shift = GetFeatureMatchShift(match, kp1_all[pic], kp2_all[pic]);
    const double spd = magnitude(shift) * kmpp / dt;
    const double dir = toDegrees(atan2(-shift.y, shift.x));

    if (std::any_of(removeSpeeds.begin(), removeSpeeds.end(), [&](const auto& remspd) { return std::abs(spd - remspd) < 1; }))
      continue;

    if (spd < data.minSpeed)
    {
      LOG_TRACE("Skipping match {}: speed {:.2f} km/s too slow", idx, spd);
      continue;
    }

    if (spd > data.maxSpeed)
    {
      LOG_WARNING("Skipping match {}: speed {:.2f} km/s too fast", idx, spd);
      continue;
    }

    static constexpr double kMinDir = -170;
    static constexpr double kMaxDir = -120;
    if (dir < kMinDir || dir > kMaxDir)
    {
      LOG_TRACE("Skipping match {}: direction {:.2f} deg off limits", idx, dir);
      continue;
    }

    minspd = std::min(spd, minspd);
    maxspd = std::max(spd, maxspd);

    shouldDraw[idx] = true;
  }

  size_t drawcounter = 1;

  for (auto it = matches_all.rbegin(); it != matches_all.rend(); ++it)
  {
    const auto& [idx, pic, match, overlap] = *it;

    if (!shouldDraw[idx])
      continue;

    const auto shift = GetFeatureMatchShift(match, kp1_all[pic], kp2_all[pic]);
    const double spd = magnitude(shift) * kmpp / dt;

    auto pts = GetFeatureMatchPoints(match, kp1_all[pic], kp2_all[pic]);
    Point2f arrStart = data.upscale * pts.first;
    Point2f arrEnd = data.upscale * pts.first + arrow_scale * out.cols / maxspd * (pts.second - pts.first);
    Point2f textpos = (arrStart + arrEnd) / 2;
    Scalar color = colorMapJet(spd, 100, 1050); // not too strong colors
    textpos.x += text_xoffset * out.cols;
    textpos.y += text_yoffset * out.cols;
    arrowedLine(out, arrStart, arrEnd, color, arrow_thickness * out.cols, LINE_AA, 0, 0.1);
    putText(out, drawSpeed ? fmt::format("{} ({:.0f})", drawcounter, spd) : fmt::format("{}", drawcounter), textpos, 1, text_scale * out.cols, color, text_thickness * out.cols, LINE_AA);

    if (data.drawOverlapCircles)
      circle(out, arrStart, data.upscale * data.overlapdistance, Scalar(0, 255, 255), text_thickness * out.cols, LINE_AA);

    drawcounter++;
  }

  LOG_INFO("Drew {} out of {} matches", drawcounter, matches_all.size());
  return out;
}

inline Mat DrawFeatureMatchArrows(const Mat& img, const std::vector<DMatch>& matches, const std::vector<KeyPoint>& kp1, const std::vector<KeyPoint>& kp2, const FeatureMatchData& data)
{
  LOG_FUNCTION("DrawFeatureMatchArrows");
  Mat out;
  cvtColor(img, out, COLOR_GRAY2BGR);

  std::vector<std::pair<DMatch, bool>> matches_filtered;
  for (const auto& match : matches)
    matches_filtered.push_back({match, true});

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(matches_filtered.begin(), matches_filtered.end(), g);

  for (auto& [match, draw] : matches_filtered)
  {
    if (!draw)
      continue;

    for (auto& [othermatch, otherdraw] : matches_filtered)
    {
      if (!otherdraw)
        continue;

      if (magnitude(kp1[match.queryIdx].pt - kp2[othermatch.queryIdx].pt) < data.overlapdistance)
        otherdraw = false;
    }
  }

  for (const auto& [match, draw] : matches_filtered)
  {
    if (!draw)
      continue;

    const auto pts = GetFeatureMatchPoints(match, kp1, kp2);
    const double spd = magnitude(GetFeatureMatchShift(match, kp1, kp2));
    const double diagonal = sqrt(sqr(img.rows) + sqr(img.cols));

    Scalar color = colorMapJet(spd, 0, 0.3 * diagonal);
    arrowedLine(out, pts.first, pts.second, color, 0.002 * out.cols, LINE_AA, 0, 0.1);
  }

  LOG_INFO("Drew {} matches", matches.size());
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

#pragma omp parallel for
  for (int pic = 1; pic < piccnt - 1; pic++)
  {
    const std::string path1 = data.path + std::to_string(pic) + ".PNG";
    const std::string path2 = data.path + std::to_string(pic + 1) + ".PNG";
    Mat img1 = imread(path1, IMREAD_GRAYSCALE);
    Mat img2 = imread(path2, IMREAD_GRAYSCALE);

    LOG_DEBUG(fmt::format("Matching images {} & {}", path1, path2));

    // detect the keypoints, compute the descriptors
    Ptr<Feature2D> detector = GetFeatureDetector(data);
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(img2, noArray(), keypoints2, descriptors2);
    keypoints1_all[pic] = keypoints1;
    keypoints2_all[pic] = keypoints2;

    // matching descriptor vectors
    auto matcher = DescriptorMatcher::create(DescriptorMatcher::MatcherType::BRUTEFORCE);
    std::vector<std::vector<DMatch>> knn_matches;
    matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

    // filter matches using the Lowe's ratio test
    std::vector<DMatch> matches;
    {
      LOG_FUNCTION("Filter matches based on ratio test");
      for (size_t i = 0; i < knn_matches.size(); i++)
        if (knn_matches[i][0].distance < data.ratioThreshold * knn_matches[i][1].distance)
          matches.push_back(knn_matches[i][0]);
    }

    // get first best fits
    {
      LOG_FUNCTION("Calculate N best matches");
      std::sort(matches.begin(), matches.end(), [&](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });
      matches = std::vector<DMatch>(matches.begin(), matches.begin() + min(data.matchcnt, (int)matches.size()));
      matches_all[pic] = matches;
    }

    if (0)
    {
      LOG_FUNCTION("Draw matches");
      Mat img_matches;
      drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar(0, 255, 255), Scalar(0, 255, 0), std::vector<char>(), DrawMatchesFlags::DEFAULT);
      showimg(img_matches, "Good matches");
    }
  }

  std::vector<std::tuple<size_t, size_t, DMatch, bool>> matches_all_serialized;
  {
    LOG_FUNCTION("Sort matches from fastest to slowest speeds");

    size_t idx = 0;
    for (size_t pic = 0; pic < piccnt - 1; pic++)
      for (const auto& match : matches_all[pic])
        matches_all_serialized.push_back({idx++, pic, match, false});

    std::sort(matches_all_serialized.begin(), matches_all_serialized.end(),
        [&](const auto& a, const auto& b)
        {
          const auto& [idx1, pic1, match1, ignore1] = a;
          const auto& [idx2, pic2, match2, ignore2] = b;
          const auto shift1 = GetFeatureMatchShift(match1, keypoints1_all[pic1], keypoints2_all[pic1]);
          const auto shift2 = GetFeatureMatchShift(match2, keypoints1_all[pic2], keypoints2_all[pic2]);
          const auto spd1 = magnitude(shift1);
          const auto spd2 = magnitude(shift2);
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

  const auto arrowsIdx = DrawFeatureMatchArrows(img_base, matches_all_serialized, keypoints1_all, keypoints2_all, data, false);
  const auto arrowsSpd = DrawFeatureMatchArrows(img_base, matches_all_serialized, keypoints1_all, keypoints2_all, data, true);

  showimg(arrowsIdx, "Match arrows idx", false, 0, 1, 1200);
  showimg(arrowsSpd, "Match arrows spd", false, 0, 1, 1200);

  saveimg("Debug/arrowsIdx.png", arrowsIdx);
  saveimg("Debug/arrowsSpd.png", arrowsSpd);
}
catch (const std::exception& e)
{
  LOG_ERROR("Feature match error: {}", e.what());
}

inline void featureMatch2pic(const FeatureMatchData& data)
try
{
  Mat img1 = imread(data.path1, IMREAD_GRAYSCALE);
  Mat img2 = imread(data.path2, IMREAD_GRAYSCALE);

  LOG_DEBUG(fmt::format("Matching images {} & {}", data.path1, data.path2));

  // detect the keypoints, compute the descriptors
  Ptr<Feature2D> detector = GetFeatureDetector(data);
  std::vector<KeyPoint> keypoints1, keypoints2;
  Mat descriptors1, descriptors2;
  detector->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
  detector->detectAndCompute(img2, noArray(), keypoints2, descriptors2);

  // matching descriptor vectors
  auto matcher = DescriptorMatcher::create(DescriptorMatcher::MatcherType::BRUTEFORCE);
  std::vector<std::vector<DMatch>> knn_matches;
  matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

  // filter matches using the Lowe's ratio test
  std::vector<DMatch> matches;
  {
    LOG_FUNCTION("Filter matches based on ratio test");
    for (size_t i = 0; i < knn_matches.size(); i++)
      if (knn_matches[i][0].distance < data.ratioThreshold * knn_matches[i][1].distance)
        matches.push_back(knn_matches[i][0]);
  }

  Plot2D::Set(fmt::format("{} I1", "featurematch"));
  Plot2D::SetSavePath(fmt::format("{}/{}_I1.png", "Debug", "featurematch"));
  Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
  Plot2D::Plot(img1);

  Plot2D::Set(fmt::format("{} I2", "featurematch"));
  Plot2D::SetSavePath(fmt::format("{}/{}_I2.png", "Debug", "featurematch"));
  Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
  Plot2D::Plot(img2);

  const auto arrows = DrawFeatureMatchArrows(img1, matches, keypoints1, keypoints2, data);
  showimg(arrows, "matches", false, 0, 1, 1024);
  saveimg("Debug/featurematch_ARR.png", arrows);
}
catch (const std::exception& e)
{
  LOG_ERROR("Feature match error: {}", e.what());
}
