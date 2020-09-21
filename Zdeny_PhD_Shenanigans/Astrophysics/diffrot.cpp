#include "stdafx.h"
#include "diffrot.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"

DiffrotResults calculateDiffrotProfile(const IterativePhaseCorrelation &ipc, FitsTime &time, const DiffrotSettings &drset)
{
  int dy = drset.vFov / (drset.ys - 1);
  std::vector<std::vector<double>> thetas2D;
  std::vector<std::vector<double>> shiftsX2D;
  std::vector<std::vector<double>> shiftsY2D;
  std::vector<std::vector<double>> omegasX2D;
  std::vector<std::vector<double>> omegasY2D;

  std::vector<double> thetas(drset.ys);
  std::vector<double> shiftsX(drset.ys);
  std::vector<double> shiftsY(drset.ys);
  std::vector<double> omegasX(drset.ys);
  std::vector<double> omegasY(drset.ys);

  std::vector<double> omegasXavg(drset.ys);
  std::vector<double> omegasYavg(drset.ys);

  thetas2D.reserve(drset.pics);
  shiftsX2D.reserve(drset.pics);
  shiftsY2D.reserve(drset.pics);
  omegasX2D.reserve(drset.pics);
  omegasY2D.reserve(drset.pics);

  FitsImage pic1, pic2;
  int lag1, lag2;

  for (int pic = 0; pic < drset.pics; ++pic)
  {
    time.advanceTime((bool)pic * (drset.sPic - drset.dPic) * drset.dSec);
    loadFitsFuzzy(pic1, time, lag1);
    time.advanceTime(drset.dPic * drset.dSec);
    loadFitsFuzzy(pic2, time, lag2);

    if (pic1.params().succload && pic2.params().succload)
    {
      if (drset.video)
      {
        Mat crop = roicrop(pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY - 0.7 * pic1.params().R, 100, 100);
        normalize(crop, crop, 0, 65535, NORM_MINMAX);
        saveimg(drset.savepath + to_string(pic) + ".png", crop, true, cv::Size(1000, 1000));
        continue;
      }

      double R = (pic1.params().R + pic2.params().R) / 2.;
      double theta0 = (pic1.params().theta0 + pic2.params().theta0) / 2.;

      int predShift = 0;
      if (drset.pred)
      {
        predShift = predictDiffrotShift(drset.dPic, drset.dSec, R);
        LOG_DEBUG_IF(drset.speak, "Predicted shift used = {}", predShift);
      }

      calculateOmegas(pic1, pic2, shiftsX, shiftsY, thetas, omegasX, omegasY, ipc, drset, R, theta0, dy, lag1, lag2, predShift);

      if (pic < 10)
      {
        LOG_SUCC_IF(drset.speak, "{} / {} estimating initial profile", pic + 1, drset.pics);
        thetas2D.emplace_back(thetas);
        shiftsX2D.emplace_back(shiftsX);
        shiftsY2D.emplace_back(shiftsY);
        omegasX2D.emplace_back(omegasX);
        omegasY2D.emplace_back(omegasY);
      }
      else
      {
        double diffX = mean(omegasX) - mean(omegasXavg);
        double diffY = mean(omegasY) - mean(omegasYavg);

        const double diffThreshX = 1; // 1
        const double diffThreshY = 2; // 2

        // filter outlier X/Y data
        if (abs(diffX) < diffThreshX && abs(diffY) < diffThreshY)
        {
          // save data
          thetas2D.emplace_back(thetas);
          shiftsX2D.emplace_back(shiftsX);
          shiftsY2D.emplace_back(shiftsY);
          omegasX2D.emplace_back(omegasX);
          omegasY2D.emplace_back(omegasY);

          // log progress
          LOG_SUCC_IF(drset.speak, "{}/{} ... diff X/Y = {:.2f}/{:.2f}, adding", pic + 1, drset.pics, diffX, diffY);
        }
        else
          LOG_ERROR_IF(drset.speak, "Abnormal profile detected, diff X = {:.2f}, diff Y = {:.2f}, skipping", diffX, diffY);
      }

      omegasXavg = meanVertical(omegasX2D);
      omegasYavg = meanVertical(omegasY2D);
    }
  }

  omegasXavg = meanVertical(omegasX2D);
  omegasYavg = meanVertical(omegasY2D);

  DiffrotResults dr;
  dr.SetData2D(thetas2D, omegasX2D, omegasY2D, shiftsX2D, shiftsY2D);
  dr.SetParams(drset.pics, drset.sPic, drset.savepath);

  return dr;
}

inline void calculateOmegas(const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &shiftsY, std::vector<double> &thetas, std::vector<double> &omegasX, std::vector<double> &omegasY, const IterativePhaseCorrelation &ipc, const DiffrotSettings &drset, double R, double theta0, double dy, int lag1, int lag2, int predShift)
{
#pragma omp parallel for
  for (int y = 0; y < drset.ys; y++)
  {
    Mat crop1 = roicrop(pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY + dy * (double)(y - drset.ys / 2) + drset.sy, ipc.GetCols(), ipc.GetRows());
    Mat crop2 = roicrop(pic2.image(), pic2.params().fitsMidX + predShift, pic2.params().fitsMidY + dy * (double)(y - drset.ys / 2) + drset.sy, ipc.GetCols(), ipc.GetRows());
    auto shift = ipc.Calculate(std::move(crop1), std::move(crop2));
    shiftsX[y] = shift.x + predShift;
    shiftsY[y] = shift.y;
  }

  if (drset.medianFilter)
  {
    filterMedian(shiftsX, drset.medianFilterSize);
    filterMedian(shiftsY, drset.medianFilterSize);
  }

  if (drset.movavgFilter)
  {
    filterMovavg(shiftsX, drset.movavgFilterSize);
    filterMovavg(shiftsY, drset.movavgFilterSize);
  }

  if (lag1 != 0 || lag2 != 0)
    LOG_DEBUG_IF(drset.speak, "Nonzero lag! lag1 = {}, lag2 = {}", lag1, lag2);

  const double dt = (double)(drset.dPic * drset.dSec + lag2 - lag1);
  for (int y = 0; y < drset.ys; y++)
  {
    thetas[y] = asin((dy * (double)(drset.ys / 2 - y) - drset.sy) / R) + theta0;
    shiftsX[y] = clamp(shiftsX[y], 0, Constants::Inf);
    omegasX[y] = asin(shiftsX[y] / (R * cos(thetas[y]))) / dt * Constants::RadPerSecToDegPerDay;
    omegasY[y] = (thetas[y] - asin(sin(thetas[y]) - shiftsY[y] / R)) / dt * Constants::RadPerSecToDegPerDay;
  }
}

void loadFitsFuzzy(FitsImage &pic, FitsTime &time, int &lag)
{
  pic.reload(time.path());

  if (pic.params().succload)
  {
    lag = 0;
    return;
  }
  else
  {
    for (int pm = 1; pm < plusminusbufer; pm++)
    {
      int shift = 0;

      if (!(pm % 2))
      {
        time.advanceTime(pm);
        shift += pm;
      }
      else
      {
        time.advanceTime(-pm);
        shift += -pm;
      }

      pic.reload(time.path());
      if (pic.params().succload)
      {
        lag = shift;
        return;
      }
    }
  }
  lag = 0;
  time.advanceTime(plusminusbufer / 2);
}
