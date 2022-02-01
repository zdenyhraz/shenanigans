#pragma once
#include "Utils/FunctionsBaseSTL.h"
#include "UtilsCV/FunctionsBaseCV.h"
#include "Fit/Polyfit.h"
#include "Fit/Nnfit.h"
#include "UtilsCV/Combinepics.h"
#include "UtilsCV/Showsave.h"
#include "UtilsCV/Export.h"
#include "Logger/Logger.h"
#include "Logger/LogFunction.h"
#include "Plot/Plot2D.h"

static constexpr i32 piccnt = 9;             // number of pics
static constexpr f64 kmpp = 696010. / 378.3; // kilometers per pixel
static constexpr f64 dt = 11.8;              // dt seconds temporally adjacent pics
static constexpr f64 arrow_scale = 12;
static constexpr f64 arrow_thickness = 0.0015;
static constexpr f64 text_scale = 0.001;
static constexpr f64 text_thickness = arrow_thickness * 1.0;
static constexpr f64 text_xoffset = 0.0075;
static constexpr f64 text_yoffset = 2 * text_xoffset;

enum FeatureType
{
  SURF,
  SIFT,
};

struct FeatureMatchData
{
  FeatureType ftype;
  f64 thresh;
  i32 matchcnt;
  f64 minSpeed;
  f64 maxSpeed;
  std::string path;
  std::string path1;
  std::string path2;
  f64 overlapdistance;
  bool drawOverlapCircles = false;
  f64 ratioThreshold = 0.7;
  f64 upscale = 1;
  bool surfExtended = false;
  bool surfUpright = false;
  i32 nOctaves = 4;
  i32 nOctaveLayers = 3;
  bool mask = true;
};

inline cv::Point2f GetFeatureMatchShift(const cv::DMatch& match, const std::vector<cv::KeyPoint>& kp1, const std::vector<cv::KeyPoint>& kp2)
{
  return kp2[match.trainIdx].pt - kp1[match.queryIdx].pt;
}

inline std::pair<cv::Point2f, cv::Point2f> GetFeatureMatchPoints(const cv::DMatch& match, const std::vector<cv::KeyPoint>& kp1, const std::vector<cv::KeyPoint>& kp2)
{
  return std::make_pair(kp1[match.queryIdx].pt, kp2[match.trainIdx].pt);
}

inline cv::Ptr<cv::Feature2D> GetFeatureDetector(const FeatureMatchData& data)
{
  switch (data.ftype)
  {
  case FeatureType::SURF:
    return cv::xfeatures2d::SURF::create(std::min(data.thresh, 500.), std::max(data.nOctaves, 1), std::max(data.nOctaveLayers, 1), data.surfExtended, data.surfUpright);
  case FeatureType::SIFT:
    return cv::SIFT::create(0, std::max(data.nOctaveLayers, 1), 0, 1e5);
  }

  throw std::runtime_error("Unknown feature type");
}

inline void ExportFeaturesToCsv(const std::string& path, const std::vector<cv::Point2f>& points, const std::vector<f64>& speeds, const std::vector<f64>& directions)
{
  std::string pth = path + "features.csv";
  std::ofstream csv(pth, std::ios::out | std::ios::trunc);
  csv << "X,Y,SPD,DIR" << std::endl;
  for (usize i = 0; i < points.size(); i++)
  {
    csv << points[i].x << "," << points[i].y << "," << speeds[i] << "," << directions[i] << std::endl;
  }
  LOG_INFO("Feature data exported to {}", std::filesystem::weakly_canonical(pth));
}

inline cv::Mat DrawFeatureMatchArrows(const cv::Mat& img, const std::vector<std::tuple<usize, usize, cv::DMatch, bool>>& matches_all, const std::vector<std::vector<cv::KeyPoint>>& kp1_all,
    const std::vector<std::vector<cv::KeyPoint>>& kp2_all, const FeatureMatchData& data, bool drawSpeed)
{
  LOG_FUNCTION("DrawFeatureMatchArrows");
  cv::Mat out;
  cvtColor(img, out, cv::COLOR_GRAY2BGR);

  if (data.upscale != 1)
    resize(out, out, cv::Size(data.upscale * out.cols, data.upscale * out.rows), 0, 0, cv::INTER_LINEAR);

  f64 minspd = std::numeric_limits<f64>::max();
  f64 maxspd = std::numeric_limits<f64>::min();
  std::vector<bool> shouldDraw(matches_all.size(), false);
  std::vector<f64> removeSpeeds = {}; // 639, 652

  if (false)
    for (i32 r = 0; r < out.rows; ++r)
      for (i32 c = 0; c < out.cols; ++c)
        if (((f32)c / img.cols + (f32)r / img.rows) / 2 < 0.5)
          out.at<cv::Vec3b>(r, c) = (out.at<cv::Vec3b>(r, c) + cv::Vec3b(0, 0, 255)) / 2;

  for (auto it = matches_all.rbegin(); it != matches_all.rend(); ++it)
  {
    const auto& [idx, pic, match, overlap] = *it;

    if (overlap)
      continue;

    const auto& point = kp1_all[pic][match.queryIdx].pt;

    if (data.mask and (((f32)point.x / img.cols + (f32)point.y / img.rows) / 2 < 0.5))
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
    const f64 spd = magnitude(shift) * kmpp / dt;
    const f64 dir = toDegrees(atan2(-shift.y, shift.x));

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

    static constexpr f64 kMinDir = -170;
    static constexpr f64 kMaxDir = -120;
    if (dir < kMinDir or dir > kMaxDir)
    {
      LOG_TRACE("Skipping match {}: direction {:.2f} deg off limits", idx, dir);
      continue;
    }

    minspd = std::min(spd, minspd);
    maxspd = std::max(spd, maxspd);

    shouldDraw[idx] = true;
  }

  usize drawcounter = 1;

  for (auto it = matches_all.rbegin(); it != matches_all.rend(); ++it)
  {
    const auto& [idx, pic, match, overlap] = *it;

    if (!shouldDraw[idx])
      continue;

    const auto shift = GetFeatureMatchShift(match, kp1_all[pic], kp2_all[pic]);
    const f64 spd = magnitude(shift) * kmpp / dt;

    auto pts = GetFeatureMatchPoints(match, kp1_all[pic], kp2_all[pic]);
    cv::Point2f arrStart = data.upscale * pts.first;
    cv::Point2f arrEnd = data.upscale * pts.first + arrow_scale * out.cols / maxspd * (pts.second - pts.first);
    cv::Point2f textpos = (arrStart + arrEnd) / 2;
    cv::Scalar color = colorMapJet(spd, 100, 1050); // not too strong colors
    textpos.x += text_xoffset * out.cols;
    textpos.y += text_yoffset * out.cols;
    arrowedLine(out, arrStart, arrEnd, color, arrow_thickness * out.cols, cv::LINE_AA, 0, 0.1);
    putText(out, drawSpeed ? fmt::format("{} ({:.0f})", drawcounter, spd) : fmt::format("{}", drawcounter), textpos, 1, text_scale * out.cols, color, text_thickness * out.cols, cv::LINE_AA);

    if (data.drawOverlapCircles)
      circle(out, arrStart, data.upscale * data.overlapdistance, cv::Scalar(0, 255, 255), text_thickness * out.cols, cv::LINE_AA);

    drawcounter++;
  }

  LOG_INFO("Drew {} out of {} matches", drawcounter, matches_all.size());
  return out;
}

inline cv::Mat DrawFeatureMatchArrows(
    const cv::Mat& img, const std::vector<cv::DMatch>& matches, const std::vector<cv::KeyPoint>& kp1, const std::vector<cv::KeyPoint>& kp2, const FeatureMatchData& data)
{
  LOG_FUNCTION("DrawFeatureMatchArrows");
  cv::Mat out;
  cvtColor(img, out, cv::COLOR_GRAY2BGR);

  std::vector<std::pair<cv::DMatch, bool>> matches_filtered;
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
    const f64 spd = magnitude(GetFeatureMatchShift(match, kp1, kp2));
    const f64 diagonal = sqrt(sqr(img.rows) + sqr(img.cols));

    cv::Scalar color = colorMapJet(spd, 0, 0.3 * diagonal);
    arrowedLine(out, pts.first, pts.second, color, 0.002 * out.cols, cv::LINE_AA, 0, 0.1);
  }

  LOG_INFO("Drew {} matches", matches.size());
  return out;
}

inline void featureMatch(const FeatureMatchData& data)
try
{
  LOG_FUNCTION("FeatureMatch");

  cv::Mat img_base = cv::imread(data.path + "5.PNG", cv::IMREAD_GRAYSCALE);
  std::vector<std::vector<cv::DMatch>> matches_all(piccnt - 1);
  std::vector<std::vector<cv::KeyPoint>> keypoints1_all(piccnt - 1);
  std::vector<std::vector<cv::KeyPoint>> keypoints2_all(piccnt - 1);

#pragma omp parallel for
  for (i32 pic = 1; pic < piccnt - 1; pic++)
  {
    const std::string path1 = data.path + std::to_string(pic) + ".PNG";
    const std::string path2 = data.path + std::to_string(pic + 1) + ".PNG";
    cv::Mat img1 = cv::imread(path1, cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread(path2, cv::IMREAD_GRAYSCALE);

    LOG_DEBUG(fmt::format("Matching images {} & {}", path1, path2));

    // detect the keypoints, compute the descriptors
    cv::Ptr<cv::Feature2D> detector = GetFeatureDetector(data);
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;
    detector->detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(img2, cv::noArray(), keypoints2, descriptors2);
    keypoints1_all[pic] = keypoints1;
    keypoints2_all[pic] = keypoints2;

    // matching descriptor vectors
    auto matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::MatcherType::BRUTEFORCE);
    std::vector<std::vector<cv::DMatch>> knn_matches;
    matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

    // filter matches using the Lowe's ratio test
    std::vector<cv::DMatch> matches;
    {
      LOG_FUNCTION("Filter matches based on ratio test");
      for (usize i = 0; i < knn_matches.size(); i++)
        if (knn_matches[i][0].distance < data.ratioThreshold * knn_matches[i][1].distance)
          matches.push_back(knn_matches[i][0]);
    }

    // get first best fits
    {
      LOG_FUNCTION("Calculate N best matches");
      std::sort(matches.begin(), matches.end(), [&](const cv::DMatch& a, const cv::DMatch& b) { return a.distance < b.distance; });
      matches = std::vector<cv::DMatch>(matches.begin(), matches.begin() + std::min(data.matchcnt, (i32)matches.size()));
      matches_all[pic] = matches;
    }

    if (0)
    {
      LOG_FUNCTION("Draw matches");
      cv::Mat img_matches;
      drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches, cv::Scalar(0, 255, 255), cv::Scalar(0, 255, 0), std::vector<char>(), cv::DrawMatchesFlags::DEFAULT);
      showimg(img_matches, "Good matches");
    }
  }

  std::vector<std::tuple<usize, usize, cv::DMatch, bool>> matches_all_serialized;
  {
    LOG_FUNCTION("Sort matches from fastest to slowest speeds");

    usize i = 0;
    for (usize pic = 0; pic < piccnt - 1; pic++)
      for (const auto& match : matches_all[pic])
        matches_all_serialized.push_back({i++, pic, match, false});

    std::sort(matches_all_serialized.begin(), matches_all_serialized.end(), [&](const auto& a, const auto& b) {
      const auto& [idx1, pic1, match1, ignore1] = a;
      const auto& [idx2, pic2, match2, ignore2] = b;
      const auto shift1 = GetFeatureMatchShift(match1, keypoints1_all[pic1], keypoints2_all[pic1]);
      const auto shift2 = GetFeatureMatchShift(match2, keypoints1_all[pic2], keypoints2_all[pic2]);
      const auto spd1 = magnitude(shift1);
      const auto spd2 = magnitude(shift2);
      return spd1 > spd2;
    });

    usize newidx = 0;
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
        const f64 distance = magnitude(shift);
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
  cv::Mat img1 = cv::imread(data.path1, cv::IMREAD_GRAYSCALE);
  cv::Mat img2 = cv::imread(data.path2, cv::IMREAD_GRAYSCALE);

  LOG_DEBUG(fmt::format("Matching images {} & {}", data.path1, data.path2));

  // detect the keypoints, compute the descriptors
  cv::Ptr<cv::Feature2D> detector = GetFeatureDetector(data);
  std::vector<cv::KeyPoint> keypoints1, keypoints2;
  cv::Mat descriptors1, descriptors2;
  detector->detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);
  detector->detectAndCompute(img2, cv::noArray(), keypoints2, descriptors2);

  // matching descriptor vectors
  auto matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::MatcherType::BRUTEFORCE);
  std::vector<std::vector<cv::DMatch>> knn_matches;
  matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

  // filter matches using the Lowe's ratio test
  std::vector<cv::DMatch> matches;
  {
    LOG_FUNCTION("Filter matches based on ratio test");
    for (usize i = 0; i < knn_matches.size(); i++)
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
