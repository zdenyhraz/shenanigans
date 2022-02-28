#pragma once

namespace Procedural
{
inline cv::Mat sinian(i32 rows, i32 cols, f64 rowsFreq, f64 colsFreq, f64 rowsShift, f64 colsShift)
{
  cv::Mat mat = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
    for (i32 c = 0; c < cols; c++)
      mat.at<f32>(r, c) = sin(colsFreq * Constants::TwoPi * ((f64)c / (cols - 1) + colsShift)) + sin(rowsFreq * Constants::TwoPi * ((f64)r / (rows - 1) + rowsShift));
  return mat;
}

inline cv::Mat gaussian(i32 rows, i32 cols, f64 rowsSigma, f64 colsSigma, f64 rowsShift, f64 colsShift)
{
  cv::Mat mat = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
  {
    for (i32 c = 0; c < cols; c++)
    {
      mat.at<f32>(r, c) = exp(-pow((f64)c / (cols - 1) - colsShift, 2) / colsSigma - pow((f64)r / (rows - 1) - rowsShift, 2) / rowsSigma);
    }
  }
  return mat;
}

inline cv::Mat procedural(i32 rows, i32 cols)
{
  usize N = 500;
  cv::Mat mat = cv::Mat::zeros(rows, cols, CV_32F);

  for (usize i = 0; i < N; i++)
  {
    f32 cx = 0.02 * RandU();
    f32 cy = 0.02 * RandU();
    f32 ratio = cx / cy;

    if (ratio > 5 or ratio < (1. / 5))
      continue;

    mat += (RandU() * gaussian(rows, cols, cx, cy, RandU(), RandU()));
  }
  normalize(mat, mat, 0, 1, cv::NORM_MINMAX);
  return mat;
}

inline cv::Mat colorlandscape(const cv::Mat& heightmap)
{
  cv::Mat mat = cv::Mat::zeros(heightmap.rows, heightmap.cols, CV_32FC3);

  for (i32 r = 0; r < heightmap.rows; r++)
  {
    for (i32 c = 0; c < heightmap.cols; c++)
    {
      auto& x = heightmap.at<f32>(r, c);

      if (x < 0.23)
      {
        // deep water
        mat.at<cv::Vec3f>(r, c)[0] = 255. / 255;
        mat.at<cv::Vec3f>(r, c)[1] = 0. / 255;
        mat.at<cv::Vec3f>(r, c)[2] = 0. / 255;
      }
      else if (x < 0.35)
      {
        // shallow water
        mat.at<cv::Vec3f>(r, c)[0] = 255. / 255;
        mat.at<cv::Vec3f>(r, c)[1] = 191. / 255;
        mat.at<cv::Vec3f>(r, c)[2] = 0. / 255;
      }
      else if (x < 0.43)
      {
        // sandy beaches
        mat.at<cv::Vec3f>(r, c)[0] = 62. / 255;
        mat.at<cv::Vec3f>(r, c)[1] = 226. / 255;
        mat.at<cv::Vec3f>(r, c)[2] = 254. / 255;
      }
      else if (x < 0.55)
      {
        // grass lands
        mat.at<cv::Vec3f>(r, c)[0] = 50. / 255;
        mat.at<cv::Vec3f>(r, c)[1] = 205. / 255;
        mat.at<cv::Vec3f>(r, c)[2] = 50. / 255;
      }
      else if (x < 0.65)
      {
        // forest
        mat.at<cv::Vec3f>(r, c)[0] = 0. / 255;
        mat.at<cv::Vec3f>(r, c)[1] = 128. / 255;
        mat.at<cv::Vec3f>(r, c)[2] = 0. / 255;
      }
      else if (x < 0.73)
      {
        // dirt
        mat.at<cv::Vec3f>(r, c)[0] = 19. / 255;
        mat.at<cv::Vec3f>(r, c)[1] = 69. / 255;
        mat.at<cv::Vec3f>(r, c)[2] = 139. / 255;
      }
      else if (x < 0.87)
      {
        // rocky hills
        mat.at<cv::Vec3f>(r, c)[0] = 0.5;
        mat.at<cv::Vec3f>(r, c)[1] = 0.5;
        mat.at<cv::Vec3f>(r, c)[2] = 0.5;
      }
      else
      {
        // snow
        mat.at<cv::Vec3f>(r, c)[0] = 0.88;
        mat.at<cv::Vec3f>(r, c)[1] = 0.88;
        mat.at<cv::Vec3f>(r, c)[2] = 0.88;
      }
    }
  }
  return mat;
}
}