#pragma once
#include "stdafx.h"
#include "optimization.h"
#include "Evolution.h"
#include "PatternSearch.h"
#include "OptimizationTestFunctions.h"

#ifdef VISUAL_OPTIMIZATION

inline cv::Mat drawFunc2D(std::function<double(vector<double>)> f, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY)
{
  cv::Mat resultMat = cv::Mat::zeros(stepsY, stepsX, CV_32F);
  int progress = 0;

#  pragma omp parallel for
  for (int r = 0; r < resultMat.rows; ++r)
  {
    for (int c = 0; c < resultMat.cols; ++c)
    {
      double x = xmin + (xmax - xmin) / (stepsX - 1) * c;
      double y = ymax - (ymax - ymin) / (stepsY - 1) * r;
      resultMat.at<float>(r, c) = f(vector<double>{x, y});
    }
#  pragma omp critical
    {
      progress++;
      cout << (double)progress / resultMat.rows * 100 << "% done." << endl;
    }
  }
  normalize(resultMat, resultMat, 0, 255, NORM_MINMAX);
  return resultMat;
}

inline void drawPoint2D(const cv::Mat &funcLandscape, cv::Point point, double stretchFactorX, double stretchFactorY, cv::Scalar CrosshairColor = Scalar(255 * 0.7, 0, 255 * 0.7))
{
  // Scalar(255 * 0.7, 0, 255 * 0.7) - magenta
  int linePxLength = max(funcLandscape.cols / 120, 1);
  int thickness = max(funcLandscape.cols / 200, 1);
  point.x *= stretchFactorX;
  point.y *= stretchFactorY;
  cv::Point NW(point.x - linePxLength, point.y - linePxLength);
  cv::Point NE(point.x + linePxLength, point.y - linePxLength);
  cv::Point SW(point.x - linePxLength, point.y + linePxLength);
  cv::Point SE(point.x + linePxLength, point.y + linePxLength);
  line(funcLandscape, NW, SE, CrosshairColor, thickness);
  line(funcLandscape, NE, SW, CrosshairColor, thickness);
}

inline void drawArrow2D(const cv::Mat &funcLandscape, cv::Point point1, cv::Point point2, double stretchFactorX, double stretchFactorY, cv::Scalar CrosshairColor = Scalar(0, 0, 255))
{
  point1.x *= stretchFactorX;
  point1.y *= stretchFactorY;
  point2.x *= stretchFactorX;
  point2.y *= stretchFactorY;
  int thickness = max(funcLandscape.cols / 400, 1);
  arrowedLine(funcLandscape, point1, point2, CrosshairColor, thickness);
}

cv::Mat drawPath2D(cv::Mat funcLandscape, vector<vector<vector<double>>> points, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY, double stretchFactorX, double stretchFactorY, bool drawArrows)
{
  cv::Mat resultMat = funcLandscape.clone();
  for (int run = 0; run < points.size(); run++)
  {
    auto colorThisRun = cv::Scalar(rand() % 256, rand() % 256, rand() % 256);
    for (int i = 0; i < points[run].size(); i++)
    {
      int col1 = (points[run][i][0] - xmin) / (xmax - xmin) * (stepsX - 1);
      int row1 = stepsY - (points[run][i][1] - ymin) / (ymax - ymin) * (stepsY - 1);
      cv::Point pointik1(col1, row1);
      if (i == 0)
      {
        if (drawArrows)
          drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, Scalar(255, 0, 0)); // start of path-B
      }
      else if (i == points[run].size() - 1)
      {
        if (drawArrows)
          drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, Scalar(0, 255, 0)); // end of path-G
      }
      else
        drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, colorThisRun);

      if (drawArrows)
      {
        if (i < (points[run].size() - 1))
        {
          int col2 = (points[run][i + 1][0] - xmin) / (xmax - xmin) * (stepsX - 1);
          int row2 = stepsY - (points[run][i + 1][1] - ymin) / (ymax - ymin) * (stepsY - 1);
          cv::Point pointik2(col2, row2);
          drawArrow2D(resultMat, pointik1, pointik2, stretchFactorX, stretchFactorY, colorThisRun);
        }
      }
    }
  }
  return resultMat;
}

inline std::vector<double> drawFuncLandscapeAndOptimize2D(std::function<double(vector<double>)> f, vector<double> lowerBounds, vector<double> upperBounds, vector<int> steps, Evolution Evo, PatternSearch Pat, bool logLandscapeOpt, bool optPat, bool optEvo, double quantileB, double quantileT, cv::Mat *landscape)
{
  cv::Mat optimizedFuncLandscapeCLR = cv::Mat::zeros(steps[0], steps[1], CV_32F);
  cv::Point2i minloc, maxloc;
  vector<double> minlocArg(2, 0);
  double stretchFactorX, stretchFactorY;
  cout << "Drawing the function landscape... " << endl;
  cv::Mat optimizedFuncLandscapeRAW = drawFunc2D(f, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1]);
  if (landscape)
    *landscape = optimizedFuncLandscapeRAW;
  // optimizedFuncLandscapeCLR = applyColorMap(optimizedFuncLandscapeRAW, quantileB, quantileT);
  cv::Mat optimizedFuncLandscapeRAWlog = optimizedFuncLandscapeRAW.clone() + Scalar::all(1.);
  log(optimizedFuncLandscapeRAWlog, optimizedFuncLandscapeRAWlog);
  normalize(optimizedFuncLandscapeRAW, optimizedFuncLandscapeRAW, 0, 255, NORM_MINMAX);
  normalize(optimizedFuncLandscapeRAWlog, optimizedFuncLandscapeRAWlog, 0, 255, NORM_MINMAX);
  cv::Mat optimizedFuncLandscapeCLRlog; //= applyColorMap(optimizedFuncLandscapeRAWlog, quantileB, quantileT); // log or not

  double minim, maxim;
  minMaxLoc(optimizedFuncLandscapeRAW, &minim, &maxim, &minloc, &maxloc);
  minlocArg[0] = lowerBounds[0] + (upperBounds[0] - lowerBounds[0]) / (steps[0] - 1) * minloc.x;
  minlocArg[1] = upperBounds[1] - (upperBounds[1] - lowerBounds[1]) / (steps[1] - 1) * minloc.y;

  int stretchSize = 1001; // odd
  stretchFactorX = (double)stretchSize / optimizedFuncLandscapeRAW.cols;
  stretchFactorY = (double)stretchSize / optimizedFuncLandscapeRAW.rows;
  resize(optimizedFuncLandscapeCLR, optimizedFuncLandscapeCLR, cv::Size(stretchSize, stretchSize), 0, 0, INTER_NEAREST);
  resize(optimizedFuncLandscapeCLRlog, optimizedFuncLandscapeCLRlog, cv::Size(stretchSize, stretchSize), 0, 0, INTER_NEAREST);
  drawPoint2D(optimizedFuncLandscapeCLR, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
  drawPoint2D(optimizedFuncLandscapeCLRlog, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
  // showimg(optimizedFuncLandscapeCLR, "optimizedFuncLandscape");
  // showimg(optimizedFuncLandscapeCLRlog, "optimizedFuncLandscape-log");
  // showEntity(minlocArg, f(minlocArg), "Result - BRUTE", true);
  cout << "done." << endl;

  if (1)
  {
    if (logLandscapeOpt)
      optimizedFuncLandscapeCLR = optimizedFuncLandscapeCLRlog;

    vector<double> resultPat = zerovect(Evo.N), resultEvo = zerovect(Evo.N);
    if (optPat)
    {
      resultPat = Pat.optimize(f);
    }
    if (optEvo)
    {
      resultEvo = Evo.Optimize(f);
    }

    // OUTPUT
    cout << endl;
    if (optPat)
      cout << "Termination reason - PAT: " << Pat.terminationReason << endl;
    if (optEvo)
      cout << "Termination reason - EVO: " << Evo.terminationReason << endl;
    if (optPat)
      cout << "Function evals - PAT: " << Pat.funEvals << " (multistartCnt: " << Pat.multistartCnt << ")" << endl;
    if (optEvo)
      cout << "Function evals - EVO: " << Evo.funEvals << endl;
    // if (optPat) showEntity(resultPat, f(resultPat), "Result - PAT", true);
    // if (optEvo) showEntity(resultEvo, f(resultEvo), "Result - EVO", true);
    // showEntity(minlocArg, f(minlocArg), "Result - BRUTE", true);
    cv::Mat optimizedFuncLandscapeWithPathPAT = optimizedFuncLandscapeCLR, optimizedFuncLandscapeWithPathEVO = optimizedFuncLandscapeCLR;
    if (optPat)
      optimizedFuncLandscapeWithPathPAT = drawPath2D(optimizedFuncLandscapeWithPathPAT, Pat.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 0);
    if (optEvo)
      optimizedFuncLandscapeWithPathEVO = drawPath2D(optimizedFuncLandscapeWithPathEVO, Evo.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 0);
    if (optPat)
      optimizedFuncLandscapeWithPathPAT = drawPath2D(optimizedFuncLandscapeWithPathPAT, Pat.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 1);
    if (optEvo)
      optimizedFuncLandscapeWithPathEVO = drawPath2D(optimizedFuncLandscapeWithPathEVO, Evo.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 1);

    if (optPat)
      drawPoint2D(optimizedFuncLandscapeWithPathPAT, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
    if (optEvo)
      drawPoint2D(optimizedFuncLandscapeWithPathEVO, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
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
  std::function<double(vector<double>)> f = OptimizationTestFunctions::Rosenbrock;
  vector<double> lowerBounds{-4.5, -4.5};
  vector<double> upperBounds{4.5, 4.5}; // odd
  vector<int> steps{501, 501};          // odd

  /*
  //meta Evo
  std::function<double(vector<double>)> f = metaOptFuncEvo;
  vector<double> lowerBounds{ 15, 0.7, 0.5, 0.5 };
  vector<double> upperBounds{ 15, 0.7, 0.5, 0.5 };//odd
  vector<int> steps{ (int)(2 * pmranges[0] + 1), (int)(2 * pmranges[0] + 1), (int)(2 * pmranges[0] + 1), 3 };//odd
  */

  Evolution Evo(lowerBounds.size());
  PatternSearch Pat(lowerBounds.size());

  Evo.lowerBounds = lowerBounds;
  Evo.upperBounds = upperBounds;
  Evo.optimalFitness = 1e-4;
  Evo.mNP = 50;

  Pat.lowerBounds = lowerBounds;
  Pat.upperBounds = upperBounds;
  Pat.optimalFitness = 1e-4;
  Pat.multistartMaxCnt = 3;

  auto Result = drawFuncLandscapeAndOptimize2D(f, lowerBounds, upperBounds, steps, Evo, Pat, 1, 1, 1, 0, 1, nullptr);

  cout << "> Done broski." << endl;
}

#endif
