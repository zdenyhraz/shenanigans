#pragma once

#include "Optimization_.h"
#include "Evolution.h"
#include "PatternSearch.h"
#include "OptimizationTestFunctions.h"

#ifdef VISUAL_OPTIMIZATION

inline cv::Mat drawFunc2D(std::function<f64(vector<f64>)> f, f64 xmin, f64 xmax, f64 ymin, f64 ymax, i32 stepsX, i32 stepsY)
{
  cv::Mat resultMat = cv::Mat::zeros(stepsY, stepsX, CV_32F);
  i32 progress = 0;

  #pragma omp parallel for
  for (i32 r = 0; r < resultMat.rows; ++r)
  {
    for (i32 c = 0; c < resultMat.cols; ++c)
    {
      f64 x = xmin + (xmax - xmin) / (stepsX - 1) * c;
      f64 y = ymax - (ymax - ymin) / (stepsY - 1) * r;
      resultMat.at<f32>(r, c) = f(vector<f64>{x, y});
    }
  #pragma omp critical
    {
      progress++;
      cout << (f64)progress / resultMat.rows * 100 << "% done." << std::endl;
    }
  }
  normalize(resultMat, resultMat, 0, 255, cv::NORM_MINMAX);
  return resultMat;
}

inline void drawPoint2D(const cv::Mat& funcLandscape, cv::Point point, f64 stretchFactorX, f64 stretchFactorY, cv::Scalar CrosshairColor = cv::Scalar(255 * 0.7, 0, 255 * 0.7))
{
  // cv::Scalar(255 * 0.7, 0, 255 * 0.7) - magenta
  i32 linePxLength = max(funcLandscape.cols / 120, 1);
  i32 thickness = max(funcLandscape.cols / 200, 1);
  point.x *= stretchFactorX;
  point.y *= stretchFactorY;
  cv::Point NW(point.x - linePxLength, point.y - linePxLength);
  cv::Point NE(point.x + linePxLength, point.y - linePxLength);
  cv::Point SW(point.x - linePxLength, point.y + linePxLength);
  cv::Point SE(point.x + linePxLength, point.y + linePxLength);
  line(funcLandscape, NW, SE, CrosshairColor, thickness);
  line(funcLandscape, NE, SW, CrosshairColor, thickness);
}

inline void drawArrow2D(const cv::Mat& funcLandscape, cv::Point point1, cv::Point point2, f64 stretchFactorX, f64 stretchFactorY, cv::Scalar CrosshairColor = cv::Scalar(0, 0, 255))
{
  point1.x *= stretchFactorX;
  point1.y *= stretchFactorY;
  point2.x *= stretchFactorX;
  point2.y *= stretchFactorY;
  i32 thickness = max(funcLandscape.cols / 400, 1);
  arrowedLine(funcLandscape, point1, point2, CrosshairColor, thickness);
}

cv::Mat drawPath2D(cv::Mat funcLandscape, vector<vector<vector<f64>>> points, f64 xmin, f64 xmax, f64 ymin, f64 ymax, i32 stepsX, i32 stepsY, f64 stretchFactorX, f64 stretchFactorY, bool drawArrows)
{
  cv::Mat resultMat = funcLandscape.clone();
  for (i32 run = 0; run < points.size(); run++)
  {
    auto colorThisRun = cv::Scalar(rand() % 256, rand() % 256, rand() % 256);
    for (usize i = 0; i < points[run].size(); i++)
    {
      i32 col1 = (points[run][i][0] - xmin) / (xmax - xmin) * (stepsX - 1);
      i32 row1 = stepsY - (points[run][i][1] - ymin) / (ymax - ymin) * (stepsY - 1);
      cv::Point pointik1(col1, row1);
      if (i == 0)
      {
        if (drawArrows)
          drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, cv::Scalar(255, 0, 0)); // start of path-B
      }
      else if (i == points[run].size() - 1)
      {
        if (drawArrows)
          drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, cv::Scalar(0, 255, 0)); // end of path-G
      }
      else
        drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, colorThisRun);

      if (drawArrows)
      {
        if (i < (points[run].size() - 1))
        {
          i32 col2 = (points[run][i + 1][0] - xmin) / (xmax - xmin) * (stepsX - 1);
          i32 row2 = stepsY - (points[run][i + 1][1] - ymin) / (ymax - ymin) * (stepsY - 1);
          cv::Point pointik2(col2, row2);
          drawArrow2D(resultMat, pointik1, pointik2, stretchFactorX, stretchFactorY, colorThisRun);
        }
      }
    }
  }
  return resultMat;
}

inline std::vector<f64> drawFuncLandscapeAndOptimize2D(std::function<f64(vector<f64>)> f, vector<f64> mLB, vector<f64> mUB, vector<i32> steps, Evolution Evo, PatternSearch Pat, bool logLandscapeOpt,
    bool optPat, bool optEvo, f64 quantileB, f64 quantileT, cv::Mat* landscape)
{
  cv::Mat optimizedFuncLandscapeCLR = cv::Mat::zeros(steps[0], steps[1], CV_32F);
  cv::Point2i minloc, maxloc;
  vector<f64> minlocArg(2, 0);
  f64 stretchFactorX, stretchFactorY;
  cout << "Drawing the function landscape... " << std::endl;
  cv::Mat optimizedFuncLandscapeRAW = drawFunc2D(f, mLB[0], mUB[0], mLB[1], mUB[1], steps[0], steps[1]);
  if (landscape)
    *landscape = optimizedFuncLandscapeRAW;
  // optimizedFuncLandscapeCLR = applyColorMap(optimizedFuncLandscapeRAW, quantileB, quantileT);
  cv::Mat optimizedFuncLandscapeRAWlog = optimizedFuncLandscapeRAW.clone() + cv::Scalar::all(1.);
  log(optimizedFuncLandscapeRAWlog, optimizedFuncLandscapeRAWlog);
  normalize(optimizedFuncLandscapeRAW, optimizedFuncLandscapeRAW, 0, 255, cv::NORM_MINMAX);
  normalize(optimizedFuncLandscapeRAWlog, optimizedFuncLandscapeRAWlog, 0, 255, cv::NORM_MINMAX);
  cv::Mat optimizedFuncLandscapeCLRlog; //= applyColorMap(optimizedFuncLandscapeRAWlog, quantileB, quantileT); // log or not

  f64 minim, maxim;
  minMaxLoc(optimizedFuncLandscapeRAW, &minim, &maxim, &minloc, &maxloc);
  minlocArg[0] = mLB[0] + (mUB[0] - mLB[0]) / (steps[0] - 1) * minloc.x;
  minlocArg[1] = mUB[1] - (mUB[1] - mLB[1]) / (steps[1] - 1) * minloc.y;

  i32 stretchSize = 1001; // odd
  stretchFactorX = (f64)stretchSize / optimizedFuncLandscapeRAW.cols;
  stretchFactorY = (f64)stretchSize / optimizedFuncLandscapeRAW.rows;
  resize(optimizedFuncLandscapeCLR, optimizedFuncLandscapeCLR, cv::Size(stretchSize, stretchSize), 0, 0, cv::INTER_NEAREST);
  resize(optimizedFuncLandscapeCLRlog, optimizedFuncLandscapeCLRlog, cv::Size(stretchSize, stretchSize), 0, 0, cv::INTER_NEAREST);
  drawPoint2D(optimizedFuncLandscapeCLR, minloc, stretchFactorX, stretchFactorY, cv::Scalar(0, 0, 255));
  drawPoint2D(optimizedFuncLandscapeCLRlog, minloc, stretchFactorX, stretchFactorY, cv::Scalar(0, 0, 255));
  // showimg(optimizedFuncLandscapeCLR, "optimizedFuncLandscape");
  // showimg(optimizedFuncLandscapeCLRlog, "optimizedFuncLandscape-log");
  // showEntity(minlocArg, f(minlocArg), "Result - BRUTE", true);
  cout << "done." << std::endl;

  if constexpr (1)
  {
    if (logLandscapeOpt)
      optimizedFuncLandscapeCLR = optimizedFuncLandscapeCLRlog;

    vector<f64> resultPat = zerovect(Evo.N), resultEvo = zerovect(Evo.N);
    if (optPat)
    {
      resultPat = Pat.optimize(f);
    }
    if (optEvo)
    {
      resultEvo = Evo.Optimize(f);
    }

    // OUTPUT
    cout << std::endl;
    if (optPat)
      cout << "Termination reason - PAT: " << Pat.terminationReason << std::endl;
    if (optEvo)
      cout << "Termination reason - EVO: " << Evo.terminationReason << std::endl;
    if (optPat)
      cout << "Function evals - PAT: " << Pat.funEvals << " (multistartCnt: " << Pat.multistartCnt << ")" << std::endl;
    if (optEvo)
      cout << "Function evals - EVO: " << Evo.funEvals << std::endl;
    // if (optPat) showEntity(resultPat, f(resultPat), "Result - PAT", true);
    // if (optEvo) showEntity(resultEvo, f(resultEvo), "Result - EVO", true);
    // showEntity(minlocArg, f(minlocArg), "Result - BRUTE", true);
    cv::Mat optimizedFuncLandscapeWithPathPAT = optimizedFuncLandscapeCLR, optimizedFuncLandscapeWithPathEVO = optimizedFuncLandscapeCLR;
    if (optPat)
      optimizedFuncLandscapeWithPathPAT = drawPath2D(optimizedFuncLandscapeWithPathPAT, Pat.visitedPoints, mLB[0], mUB[0], mLB[1], mUB[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 0);
    if (optEvo)
      optimizedFuncLandscapeWithPathEVO = drawPath2D(optimizedFuncLandscapeWithPathEVO, Evo.visitedPoints, mLB[0], mUB[0], mLB[1], mUB[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 0);
    if (optPat)
      optimizedFuncLandscapeWithPathPAT = drawPath2D(optimizedFuncLandscapeWithPathPAT, Pat.visitedPoints, mLB[0], mUB[0], mLB[1], mUB[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 1);
    if (optEvo)
      optimizedFuncLandscapeWithPathEVO = drawPath2D(optimizedFuncLandscapeWithPathEVO, Evo.visitedPoints, mLB[0], mUB[0], mLB[1], mUB[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 1);

    if (optPat)
      drawPoint2D(optimizedFuncLandscapeWithPathPAT, minloc, stretchFactorX, stretchFactorY, cv::Scalar(0, 0, 255));
    if (optEvo)
      drawPoint2D(optimizedFuncLandscapeWithPathEVO, minloc, stretchFactorX, stretchFactorY, cv::Scalar(0, 0, 255));
    if (optPat)
      showimg(optimizedFuncLandscapeWithPathPAT, "optimizedFuncLandscapeWithPath - PAT");
    if (optEvo)
      showimg(optimizedFuncLandscapeWithPathEVO, "optimizedFuncLandscapeWithPath - EVO");
  }
  return minlocArg;
}

void optimizeWithLandscapeDebug()
{
  // normal
  std::function<f64(vector<f64>)> f = OptimizationTestFunctions::Rosenbrock;
  vector<f64> mLB{-4.5, -4.5};
  vector<f64> mUB{4.5, 4.5};   // odd
  vector<i32> steps{501, 501}; // odd

  /*
  //meta Evo
  std::function<f64(vector<f64>)> f = metaOptFuncEvo;
  vector<f64> mLB{ 15, 0.7, 0.5, 0.5 };
  vector<f64> mUB{ 15, 0.7, 0.5, 0.5 };//odd
  vector<i32> steps{ (i32)(2 * pmranges[0] + 1), (i32)(2 * pmranges[0] + 1), (i32)(2 * pmranges[0] + 1), 3 };//odd
  */

  Evolution Evo(mLB.size());
  PatternSearch Pat(mLB.size());

  Evo.mLB = mLB;
  Evo.mUB = mUB;
  Evo.mOptimalFitness = 1e-4;
  Evo.mNP = 50;

  Pat.mLB = mLB;
  Pat.mUB = mUB;
  Pat.mOptimalFitness = 1e-4;
  Pat.multistartMaxCnt = 3;

  auto Result = drawFuncLandscapeAndOptimize2D(f, mLB, mUB, steps, Evo, Pat, 1, 1, 1, 0, 1, nullptr);

  cout << "> Done broski." << std::endl;
}

#endif
