
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"
#include "Utils/vectmat.h"
#include "diffrot.h"

DiffrotResults calculateDiffrotProfile(const IterativePhaseCorrelation<>& ipc, FitsTime& time, const DiffrotSettings& drset)
{
  i32 dy = drset.vFov / (drset.ys - 1);
  std::vector<std::vector<f64>> thetas2D;
  std::vector<std::vector<f64>> shiftsX2D;
  std::vector<std::vector<f64>> shiftsY2D;
  std::vector<std::vector<f64>> omegasX2D;
  std::vector<std::vector<f64>> omegasY2D;

  std::vector<f64> thetas(drset.ys);
  std::vector<f64> shiftsX(drset.ys);
  std::vector<f64> shiftsY(drset.ys);
  std::vector<f64> omegasX(drset.ys);
  std::vector<f64> omegasY(drset.ys);

  std::vector<f64> omegasXavg(drset.ys);
  std::vector<f64> omegasYavg(drset.ys);

  thetas2D.reserve(drset.pics);
  shiftsX2D.reserve(drset.pics);
  shiftsY2D.reserve(drset.pics);
  omegasX2D.reserve(drset.pics);
  omegasY2D.reserve(drset.pics);

  FitsImage pic1, pic2;
  i32 lag1, lag2;

  for (i32 pic = 0; pic < drset.pics; ++pic)
  {
    time.advanceTime((bool)pic * (drset.sPic - drset.dPic) * drset.dSec);
    loadFitsFuzzy(pic1, time, lag1);
    time.advanceTime(drset.dPic * drset.dSec);
    loadFitsFuzzy(pic2, time, lag2);

    if (pic1.params().succload and pic2.params().succload and pic1.params().succParamCorrection and pic2.params().succParamCorrection)
    {
      if (drset.video)
      {
        cv::Mat crop = roicrop(pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY - 0.7 * pic1.params().R, 100, 100);
        normalize(crop, crop, 0, 65535, cv::NORM_MINMAX);
        saveimg(drset.savepath + std::to_string(pic) + ".png", crop, true, cv::Size(1000, 1000));
        continue;
      }

      f64 R = (pic1.params().R + pic2.params().R) / 2.;
      f64 theta0 = (pic1.params().theta0 + pic2.params().theta0) / 2.;

      i32 predShift = 0;
      if (drset.pred)
      {
        predShift = predictDiffrotShift(drset.dPic, drset.dSec, R);
        if (drset.speak)
          LOG_DEBUG("Predicted shift used = {}", predShift);
      }

      calculateOmegas(pic1, pic2, shiftsX, shiftsY, thetas, omegasX, omegasY, ipc, drset, R, theta0, dy, lag1, lag2, predShift);

      if (pic < 10)
      {
        if (drset.speak)
          LOG_SUCCESS("{} / {} estimating initial profile", pic + 1, drset.pics);
        thetas2D.emplace_back(thetas);
        shiftsX2D.emplace_back(shiftsX);
        shiftsY2D.emplace_back(shiftsY);
        omegasX2D.emplace_back(omegasX);
        omegasY2D.emplace_back(omegasY);
      }
      else
      {
        f64 diffX = mean(omegasX) - mean(omegasXavg);
        f64 diffY = mean(omegasY) - mean(omegasYavg);

        const f64 diffThreshX = 1; // 1
        const f64 diffThreshY = 2; // 2

        // filter outlier X/Y data
        if (abs(diffX) < diffThreshX and abs(diffY) < diffThreshY)
        {
          // save data
          thetas2D.emplace_back(thetas);
          shiftsX2D.emplace_back(shiftsX);
          shiftsY2D.emplace_back(shiftsY);
          omegasX2D.emplace_back(omegasX);
          omegasY2D.emplace_back(omegasY);

          // log progress
          if (drset.speak)
            LOG_SUCCESS("{}/{} ... diff X/Y = {:.2f}/{:.2f}, adding", pic + 1, drset.pics, diffX, diffY);
        }
        else if (drset.speak)
          LOG_ERROR("Abnormal profile detected, diff X = {:.2f}, diff Y = {:.2f}, skipping", diffX, diffY);
      }

      omegasXavg = meanVertical(omegasX2D);
      omegasYavg = meanVertical(omegasY2D);
    }
  }

  DiffrotResults dr;
  dr.SetData2D(thetas2D, omegasX2D, omegasY2D, shiftsX2D, shiftsY2D);
  dr.SetParams(drset.pics, drset.sPic, drset.savepath);

  return dr;
}

inline void calculateOmegas(const FitsImage& pic1, const FitsImage& pic2, std::vector<f64>& shiftsX, std::vector<f64>& shiftsY, std::vector<f64>& thetas, std::vector<f64>& omegasX,
    std::vector<f64>& omegasY, const IterativePhaseCorrelation<>& ipc, const DiffrotSettings& drset, f64 R, f64 theta0, f64 dy, i32 lag1, i32 lag2, i32 predShift)
{
#pragma omp parallel for
  for (i32 y = 0; y < drset.ys; y++)
  {
    cv::Mat crop1 = roicrop(pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY + dy * (f64)(y - drset.ys / 2) + drset.sy, ipc.GetCols(), ipc.GetRows());
    cv::Mat crop2 = roicrop(pic2.image(), pic2.params().fitsMidX + predShift, pic2.params().fitsMidY + dy * (f64)(y - drset.ys / 2) + drset.sy, ipc.GetCols(), ipc.GetRows());
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

  if (lag1 != 0 or lag2 != 0)
    if (drset.speak)
      LOG_DEBUG("Nonzero lag! lag1 = {}, lag2 = {}", lag1, lag2);

  const f64 dt = (f64)(drset.dPic * drset.dSec + lag2 - lag1);
  for (i32 y = 0; y < drset.ys; y++)
  {
    thetas[y] = asin((dy * (f64)(drset.ys / 2 - y) - drset.sy) / R) + theta0;
    shiftsX[y] = clamp(shiftsX[y], 0, Constants::Inf);
    omegasX[y] = asin(shiftsX[y] / (R * cos(thetas[y]))) / dt * Constants::RadPerSecToDegPerDay;
    omegasY[y] = (thetas[y] - asin(sin(thetas[y]) - shiftsY[y] / R)) / dt * Constants::RadPerSecToDegPerDay;
  }
}

void loadFitsFuzzy(FitsImage& pic, FitsTime& time, i32& lag)
{
  pic.reload(time.path());

  if (pic.params().succload)
  {
    lag = 0;
    return;
  }
  else
  {
    for (i32 pm = 1; pm < plusminusbufer; pm++)
    {
      i32 shift = 0;

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
