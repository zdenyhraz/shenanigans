#pragma once
#include "Fit/Polyfit.h"
#include "Fit/Trigfit.h"

inline f64 predictDiffrotProfile(f64 theta, f64 A, f64 B, f64 C = 0)
{
  return (A + B * pow(sin(theta), 2) + C * pow(sin(theta), 4));
}

inline i32 predictDiffrotShift(i32 dPic, i32 dSec, f64 R)
{
  f64 degPerDay = predictDiffrotProfile(0.0, 14.296, -1.847, -2.615);
  f64 days = dPic * dSec / Constants::SecondsInDay;
  f64 degs = degPerDay * days;
  return R * sin(degs / Constants::Rad);
}

class DiffrotResults
{
public:
  bool calculated = false;
  std::string saveDir;

  void ShowResults(i32 medianSize, f64 sigma, f64 quanBot = 0, f64 quanTop = 1)
  {
    if (!calculated)
      LOG_ERROR("Diffrot results not yet calculated!");

    CalculateMainInterp1D();
    CalculateMainInterp2D();
    CalculateMedianFilters(medianSize);
    CalculateAxisLimits();
    CalculatePredics();
    CalculateErrors();
    CalculateNS();
    CalculateFitCoeffs();

    // diffrot profiles
    /*
      Plot1D::Plot(toDegrees(Thetas), {polyfit(Thetas, OmegasX, 2), OmegasX, PredicXs[0], PredicXs[1]}, "diffrot profile X", "solar latitude [deg]", "west-east flow speed [deg/day]",
                   {"polyfit2", "average", "Derek A. Lamb (2017)", "Howard et al. (1983)"}, {QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 2), QPen(Plot::red, 2)},
                   saveDir + "1DXs" + to_string(SourceStride) + ".png");
      Plot1D::Plot(toDegrees(Thetas), {polyfit(Thetas, OmegasY, 3), OmegasY}, "diffrot profile Y", "solar latitude [deg]", "north-south flow speed [deg/day]", {"polyfit3", "average"},
                   {QPen(Plot::black, 3), QPen(Plot::green, 1.5)}, saveDir + "1DYs" + to_string(SourceStride) + ".png"); // rgb(119, 136, 153)

      // diffrot profiles NS
      Plot1D::Plot(toDegrees(ThetasNS), {sin2sin4fit(ThetasNS, OmegasXavgN), sin2sin4fit(ThetasNS, OmegasXavgS), OmegasXavgN, OmegasXavgS, PredicXsNS[0], PredicXsNS[1]}, "diffrot profile NS X",
                   "absolute solar latitude [deg]", "west-east flow speed [deg/day]",
                   {"trigfit North", "trigfit South", "average North", "average South", "Derek A. Lamb (2017)", "Howard et al. (1983)", "Derek N", "Derek S"},
                   {QPen(Plot::blue, 3), QPen(Plot::red, 3), QPen(Plot::blue, 1.5), QPen(Plot::red, 1.5), QPen(Plot::green, 2), QPen(Plot::black, 2), QPen(Plot::blue, 2), QPen(Plot::red, 2)},
                   saveDir + "1DNSXs" + to_string(SourceStride) + ".png");
      Plot1D::Plot(toDegrees(ThetasNS), {sin2sin4fit(ThetasNS, OmegasYavgN), sin2sin4fit(ThetasNS, OmegasYavgS), OmegasYavgN, OmegasYavgS}, "diffrot profile NS Y", "absolute solar latitude [deg]",
                   "north-south flow speed [deg/day]", {"trigfit North", "trigfit South", "average North", "average South"},
                   {QPen(Plot::blue, 3), QPen(Plot::red, 3), QPen(Plot::blue, 1.5), QPen(Plot::red, 1.5)}, saveDir + "1DNSYs" + to_string(SourceStride) + ".png");

      // shifts profiles
      Plot1D::Plot(toDegrees(Thetas), {polyfit(Thetas, ShiftsX, 2), ShiftsX, ShiftsXErrorsBot, ShiftsXErrorsTop}, "shifts profile X", "solar latitude [deg]", "west-east image shift [px]",
                   {"polyfit2", "average", "average - stdev", "average + stdev"}, {QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 0.75), QPen(Plot::red, 0.75)},
                   saveDir + "1DsXs" + to_string(SourceStride) + ".png");
      Plot1D::Plot(toDegrees(Thetas), {polyfit(Thetas, ShiftsY, 3), ShiftsY, ShiftsYErrorsBot, ShiftsYErrorsTop}, "shifts profile Y", "solar latitude [deg]", "north-south image shift [px]",
                   {"polyfit3", "average", "average - stdev", "average + stdev"}, {QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 0.75), QPen(Plot::red, 0.75)},
                   saveDir + "1DsYs" + to_string(SourceStride) + ".png");

      // flows ratio1
      Plot2D::Plot(applyQuantile(FlowX, quanBot, quanTop), "diffrot flow X", "time [days]", "solar latitude [deg]", "west-east flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta),
                   toDegrees(EndTheta), colRowRatio1, saveDir + "2DXm" + to_string(medianSize) + "r1s" + to_string(SourceStride) + ".png");
      Plot2D::Plot(applyQuantile(FlowY, quanBot, quanTop), "diffrot flow Y", "time [days]", "solar latitude [deg]", "north-south flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta),
                   toDegrees(EndTheta), colRowRatio1, saveDir + "2DYm" + to_string(medianSize) + "r1s" + to_string(SourceStride) + ".png");

      // flows ratio2
      Plot2D::Plot(applyQuantile(FlowX, quanBot, quanTop), "diffrot flow X r", "time [days]", "solar latitude [deg]", "west-east flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta),
                   toDegrees(EndTheta), colRowRatio2, saveDir + "2DXm" + to_string(medianSize) + "r2s" + to_string(SourceStride) + ".png");
      Plot2D::Plot(applyQuantile(FlowY, quanBot, quanTop), "diffrot flow Y r", "time [days]", "solar latitude [deg]", "north-south flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta),
                   toDegrees(EndTheta), colRowRatio2, saveDir + "2DYm" + to_string(medianSize) + "r2s" + to_string(SourceStride) + ".png");
           */

    LOG_INFO("Predic error = {}", GetError());
  }

  f64 GetError()
  {
    // used for optimizing - closest profile to literature profiles
    CalculateMainInterp1D();
    CalculatePredics();
    f64 error = 0;
    usize ycount = OmegasX.size();
    const auto& mycurve = OmegasX;
    const auto& myfit = polyfit(Thetas, OmegasX, 2);
    const auto& targetcurve = 0.5 * (PredicXs[0] + PredicXs[1]);

    for (usize y = 0; y < ycount; ++y)
      error += 0.8 * std::pow(myfit[y] - targetcurve[y], 2) + 0.2 * std::pow(mycurve[y] - targetcurve[y], 2);

    return sqrt(error / ycount);
  }

  static f64 Interpolate(const std::vector<f64>& xs, const std::vector<f64>& ys, f64 x)
  {
    if (xs.front() <= xs.back()) // ascending x
    {
      if (x <= xs.front())
        return ys.front();

      if (x >= xs.back())
        return ys.back();

      for (usize i = 0; i < xs.size() - 1; ++i)
        if (xs[i] <= x and xs[i + 1] > x)
          return ys[i] + (x - xs[i]) / (xs[i + 1] - xs[i]) * (ys[i + 1] - ys[i]);
    }

    if (xs.front() > xs.back()) // descending x
    {
      if (x >= xs.front())
        return ys.front();

      if (x <= xs.back())
        return ys.back();

      for (usize i = 0; i < xs.size() - 1; ++i)
        if (xs[i] > x and xs[i + 1] <= x)
          return ys[i + 1] + (x - xs[i + 1]) / (xs[i] - xs[i + 1]) * (ys[i] - ys[i + 1]);
    }

    LOG_ERROR("Interpolation fault");
    throw; // should never get here
  }

  static f64 Interpolate(const std::vector<std::vector<f64>>& xs, const std::vector<std::vector<f64>>& ys, f64 x)
  {
    const i32 ps = xs.size();
    f64 mean = 0;

    for (i32 p = 0; p < ps; ++p)
      mean += Interpolate(xs[p], ys[p], x);

    return mean / ps;
  }

  void SetData2D(const std::vector<std::vector<f64>>& thetas2D, const std::vector<std::vector<f64>>& omegasX, const std::vector<std::vector<f64>>& omegasY,
      const std::vector<std::vector<f64>>& shiftsX, const std::vector<std::vector<f64>>& shiftsY)
  {
    SourceThetas = thetas2D;
    SourceShiftsX = shiftsX;
    SourceShiftsY = shiftsY;
    SourceOmegasX = omegasX;
    SourceOmegasY = omegasY;
    Data2DSet = true;
    UpdateCalculated();
  }

  void SetParams(i32 pics, i32 stride, std::string savepath)
  {
    SourcePics = pics;
    SourceStride = stride;
    saveDir = savepath;
    ParamsSet = true;
    UpdateCalculated();
  }

  auto GetVecs2D(std::vector<std::vector<f64>>& sourceThetas, std::vector<std::vector<f64>>& sourceShiftsX, std::vector<std::vector<f64>>& sourceShiftsY, std::vector<std::vector<f64>>& sourceOmegasX,
      std::vector<std::vector<f64>>& sourceOmegasY) const
  {
    sourceThetas = SourceThetas;
    sourceShiftsX = SourceShiftsX;
    sourceShiftsY = SourceShiftsY;
    sourceOmegasX = SourceOmegasX;
    sourceOmegasY = SourceOmegasY;
  }

  auto GetParams(i32& sourcePics, i32& sourceStride) const
  {
    sourcePics = SourcePics;
    sourceStride = SourceStride;
  }

  void SetVecs2DRaw(const std::vector<std::vector<f64>>& sourceThetas, const std::vector<std::vector<f64>>& sourceShiftsX, const std::vector<std::vector<f64>>& sourceShiftsY,
      const std::vector<std::vector<f64>>& sourceOmegasX, const std::vector<std::vector<f64>>& sourceOmegasY)
  {
    SourceThetas = sourceThetas;
    SourceShiftsX = sourceShiftsX;
    SourceShiftsY = sourceShiftsY;
    SourceOmegasX = sourceOmegasX;
    SourceOmegasY = sourceOmegasY;
  }

  void SetParamsRaw(i32 sourcePics, i32 sourceStride)
  {
    SourcePics = sourcePics;
    SourceStride = sourceStride;
  }

private:
  // source data
  i32 SourcePics;
  i32 SourceStride;
  std::vector<std::vector<f64>> SourceThetas;
  std::vector<std::vector<f64>> SourceShiftsX;
  std::vector<std::vector<f64>> SourceShiftsY;
  std::vector<std::vector<f64>> SourceOmegasX;
  std::vector<std::vector<f64>> SourceOmegasY;

  // internal data
  std::vector<f64> Thetas;
  std::vector<f64> ShiftsX;
  std::vector<f64> ShiftsY;
  std::vector<f64> OmegasX;
  std::vector<f64> OmegasY;
  cv::Mat FlowX;
  cv::Mat FlowY;
  bool Data2DSet = false;
  bool ParamsSet = false;
  static constexpr f64 colRowRatio1 = 2;
  static constexpr f64 colRowRatio2 = 1.5;
  static constexpr i32 predicCnt = 4;
  std::vector<std::vector<f64>> PredicXs;
  std::vector<f64> ShiftsXErrors;
  std::vector<f64> ShiftsYErrors;
  std::vector<f64> ShiftsXErrorsBot;
  std::vector<f64> ShiftsXErrorsTop;
  std::vector<f64> ShiftsYErrorsBot;
  std::vector<f64> ShiftsYErrorsTop;
  std::vector<std::vector<f64>> PredicXsNS;
  std::vector<f64> ThetasNS;
  std::vector<f64> OmegasXavgN;
  std::vector<f64> OmegasXavgS;
  std::vector<f64> OmegasYavgN;
  std::vector<f64> OmegasYavgS;
  f64 StartTime;
  f64 EndTime;
  f64 StartTheta;
  f64 EndTheta;

  void CalculateMainInterp1D()
  {
    i32 ps = SourceThetas.size();
    i32 ys = SourceThetas[0].size();
    Thetas.resize(ys);
    ShiftsX.resize(ys);
    ShiftsY.resize(ys);
    OmegasX.resize(ys);
    OmegasY.resize(ys);

    // calculate theta range
    f64 thetaRange = Constants::Inf;
    for (i32 p = 0; p < ps; ++p)
      if (SourceThetas[p].front() < thetaRange)
        thetaRange = SourceThetas[p].front();
    f64 dth = 2. * thetaRange / (ys - 1);

    // interpolate shifts and omegas based on equidistant theta
    for (i32 y = 0; y < ys; ++y)
    {
      Thetas[y] = thetaRange - y * dth;
      ShiftsX[y] = Interpolate(SourceThetas, SourceShiftsX, Thetas[y]);
      ShiftsY[y] = Interpolate(SourceThetas, SourceShiftsY, Thetas[y]);
      OmegasX[y] = Interpolate(SourceThetas, SourceOmegasX, Thetas[y]);
      OmegasY[y] = Interpolate(SourceThetas, SourceOmegasY, Thetas[y]);
    }
  }

  void CalculateMainInterp2D()
  {
    i32 ps = SourceThetas.size();
    i32 ys = SourceThetas[0].size();

    FlowX = cv::Mat::zeros(ys, ps, CV_32F);
    FlowY = cv::Mat::zeros(ys, ps, CV_32F);

    for (i32 y = 0; y < ys; ++y)
    {
      for (i32 p = 0; p < ps; ++p)
      {
        FlowX.at<f32>(y, ps - 1 - p) = Interpolate(SourceThetas[p], SourceOmegasX[p], Thetas[y]);
        FlowY.at<f32>(y, ps - 1 - p) = Interpolate(SourceThetas[p], SourceOmegasY[p], Thetas[y]);
      }
    }
  }

  void UpdateCalculated() { calculated = Data2DSet and ParamsSet; }

  void CalculateMedianFilters(i32 medianSize)
  {
    if (medianSize)
    {
      for (i32 med = 3; med <= std::min(medianSize, 7); med += 2)
      {
        medianBlur(FlowX, FlowX, med);
        medianBlur(FlowY, FlowY, med);
      }
    }
  }

  void CalculateAxisLimits()
  {
    StartTime = 0;
    EndTime = (f64)(SourcePics - 1) * SourceStride * 45 / 60 / 60 / 24;
    StartTheta = Thetas.front();
    EndTheta = Thetas.back();
  }

  void CalculatePredics()
  {
    PredicXs = zerovect2(predicCnt, Thetas.size(), 0.);
    for (usize i = 0; i < Thetas.size(); i++)
    {
      PredicXs[0][i] = predictDiffrotProfile(Thetas[i], 14.296, -1.847, -2.615); // Derek A. Lamb (2017)
      PredicXs[1][i] = predictDiffrotProfile(Thetas[i], 14.192, -1.70, -2.36);   // Howard et al. (1983)
      PredicXs[2][i] = predictDiffrotProfile(Thetas[i], 14.299, -2.124, -2.382); // Derek A. Lamb (2017) N
      PredicXs[3][i] = predictDiffrotProfile(Thetas[i], 14.292, -1.584, -2.938); // Derek A. Lamb (2017) S
    }
  }

  void CalculateErrors()
  {
    ShiftsXErrors = getStandardDeviationsVertical(SourceShiftsX);
    ShiftsYErrors = getStandardDeviationsVertical(SourceShiftsY);
    ShiftsXErrorsBot = ShiftsX - ShiftsXErrors;
    ShiftsYErrorsBot = ShiftsY - ShiftsYErrors;
    ShiftsXErrorsTop = ShiftsX + ShiftsXErrors;
    ShiftsYErrorsTop = ShiftsY + ShiftsYErrors;
  }

  void CalculateNS()
  {
    PredicXsNS.resize(predicCnt);

    i32 zeroidx = 0;
    for (usize i = 0; i < Thetas.size() - 2; ++i)
    {
      if ((Thetas[i] > 0 and Thetas[i + 1] < 0) or Thetas[i] == 0)
      {
        zeroidx = i;
        LOG_INFO("Diffrot NS zeroidx = {}, ({:.2f} => {:.2f})", zeroidx, toDegrees(Thetas[i]), toDegrees(Thetas[i + 1]));
        break;
      }
    }

    // north hemisphere
    ThetasNS = std::vector<f64>(Thetas.begin(), Thetas.begin() + zeroidx + 1);
    PredicXsNS[0] = std::vector<f64>(PredicXs[0].begin(), PredicXs[0].begin() + zeroidx + 1);
    PredicXsNS[1] = std::vector<f64>(PredicXs[1].begin(), PredicXs[1].begin() + zeroidx + 1);
    PredicXsNS[2] = std::vector<f64>(PredicXs[2].begin(), PredicXs[2].begin() + zeroidx + 1);
    PredicXsNS[3] = std::vector<f64>(PredicXs[3].begin(), PredicXs[3].begin() + zeroidx + 1);
    OmegasXavgN = std::vector<f64>(OmegasX.begin(), OmegasX.begin() + zeroidx + 1);
    OmegasYavgN = std::vector<f64>(OmegasY.begin(), OmegasY.begin() + zeroidx + 1);

    LOG_INFO("<NS> First (max) North theta = {:.2f}", toDegrees(ThetasNS.front()));
    LOG_INFO("<NS> Last (min) North theta = {:.2f}", toDegrees(ThetasNS.back()));

    // south hemisphere
    OmegasXavgS = std::vector<f64>(OmegasX.begin() + zeroidx, OmegasX.begin() + 2 * zeroidx + 1);
    OmegasYavgS = std::vector<f64>(OmegasY.begin() + zeroidx, OmegasY.begin() + 2 * zeroidx + 1);

    std::reverse(OmegasXavgS.begin(), OmegasXavgS.end());
    std::reverse(OmegasYavgS.begin(), OmegasYavgS.end());

    if (OmegasXavgN.size() != OmegasXavgS.size() or OmegasYavgN.size() != OmegasYavgS.size())
    {
      LOG_ERROR("<NS> NS sizes mismatch: {} != {} & {} != {}", OmegasXavgN.size(), OmegasXavgS.size(), OmegasYavgN.size(), OmegasYavgS.size());
      throw;
    }

    if (OmegasXavgN.back() != OmegasXavgS.back())
    {
      LOG_ERROR("<NS> OmegasNSX are not equal at equator: {} != {}", OmegasXavgN.back(), OmegasXavgS.back());
      throw;
    }
    else
      LOG_INFO("<NS> NS equator values X match: {:.2f} = {:.2f}", OmegasXavgN.back(), OmegasXavgS.back());

    if (OmegasYavgN.back() != OmegasYavgS.back())
    {
      LOG_ERROR("<NS> OmegasNSY are not equal at equator: {} != {}", OmegasYavgN.back(), OmegasYavgS.back());
      throw;
    }
    else
      LOG_INFO("<NS> NS equator values Y match: {:.2f} = {:.2f}", OmegasYavgN.back(), OmegasYavgS.back());
  }

  void CalculateFitCoeffs()
  {
    // XY both
    LogFitCoeffs("XcoeffsPoly2", polyfitCoeffs(Thetas, OmegasX, 2));
    LogFitCoeffs("YcoeffsPoly3", polyfitCoeffs(Thetas, OmegasY, 3));
    LogFitCoeffs("XcoeffsTrig", sin2sin4fitCoeffs(Thetas, OmegasX));

    // X NS
    LogFitCoeffs("XcoeffsTrigN", sin2sin4fitCoeffs(ThetasNS, OmegasXavgN));
    LogFitCoeffs("XcoeffsTrigS", sin2sin4fitCoeffs(ThetasNS, OmegasXavgS));

    // Y NS
    LogFitCoeffs("YcoeffsTrigN", sin2sin4fitCoeffs(ThetasNS, OmegasYavgN));
    LogFitCoeffs("YcoeffsTrigS", sin2sin4fitCoeffs(ThetasNS, OmegasYavgS));
  }

  void LogFitCoeffs(const std::string& fitname, const std::vector<f64>& coeffs)
  {
    for (usize i = 0; i < coeffs.size(); i++)
      LOG_DEBUG("{} fit coefficient {} = {:.2f}", fitname, (char)('A' + i), coeffs[i]);
  }
};
