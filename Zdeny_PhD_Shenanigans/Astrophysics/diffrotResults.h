#pragma once
#include "Core/functionsBaseCV.h"
#include "Fit/polyfit.h"
#include "Fit/trigfit.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"
#include "stdafx.h"

inline double predictDiffrotProfile(double theta, double A, double B, double C = 0) { return (A + B * pow(sin(theta), 2) + C * pow(sin(theta), 4)); }

inline int predictDiffrotShift(int dPic, int dSec, double R)
{
  double degPerDay = predictDiffrotProfile(0.0, 14.296, -1.847, -2.615);
  double days = dPic * dSec / Constants::SecondsInDay;
  double degs = degPerDay * days;
  return R * sin(degs / Constants::Rad);
}

class DiffrotResults
{
public:
  bool calculated = false;
  std::string saveDir;

  void ShowResults(int medianSize, double sigma, double quanBot = 0, double quanTop = 1)
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
    Plot1D::plot(toDegrees(Thetas), {polyfit(Thetas, OmegasX, 2), OmegasX, PredicXs[0], PredicXs[1]}, "diffrot profile X", "solar latitude [deg]", "west-east flow speed [deg/day]", {"polyfit2", "average", "Derek A. Lamb (2017)", "Howard et al. (1983)"}, {QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 2), QPen(Plot::red, 2)}, saveDir + "1DXs" + to_string(SourceStride) + ".png");
    Plot1D::plot(toDegrees(Thetas), {polyfit(Thetas, OmegasY, 3), OmegasY}, "diffrot profile Y", "solar latitude [deg]", "north-south flow speed [deg/day]", {"polyfit3", "average"}, {QPen(Plot::black, 3), QPen(Plot::green, 1.5)}, saveDir + "1DYs" + to_string(SourceStride) + ".png"); // rgb(119, 136, 153)

    // diffrot profiles NS
    Plot1D::plot(toDegrees(ThetasNS), {sin2sin4fit(ThetasNS, OmegasXavgN), sin2sin4fit(ThetasNS, OmegasXavgS), OmegasXavgN, OmegasXavgS, PredicXsNS[0], PredicXsNS[1]}, "diffrot profile NS X", "absolute solar latitude [deg]", "west-east flow speed [deg/day]", {"trigfit North", "trigfit South", "average North", "average South", "Derek A. Lamb (2017)", "Howard et al. (1983)"}, {QPen(Plot::blue, 3), QPen(Plot::red, 3), QPen(Plot::blue, 1.5), QPen(Plot::red, 1.5), QPen(Plot::green, 2), QPen(Plot::black, 2)}, saveDir + "1DNSXs" + to_string(SourceStride) + ".png");
    Plot1D::plot(toDegrees(ThetasNS), {sin2sin4fit(ThetasNS, OmegasYavgN), sin2sin4fit(ThetasNS, OmegasYavgS), OmegasYavgN, OmegasYavgS}, "diffrot profile NS Y", "absolute solar latitude [deg]", "north-south flow speed [deg/day]", {"trigfit North", "trigfit South", "average North", "average South"}, {QPen(Plot::blue, 3), QPen(Plot::red, 3), QPen(Plot::blue, 1.5), QPen(Plot::red, 1.5)}, saveDir + "1DNSYs" + to_string(SourceStride) + ".png");

    // shifts profiles
    Plot1D::plot(toDegrees(Thetas), {polyfit(Thetas, ShiftsX, 2), ShiftsX, ShiftsXErrorsBot, ShiftsXErrorsTop}, "shifts profile X", "solar latitude [deg]", "west-east image shift [px]", {"polyfit2", "average", "average - stdev", "average + stdev"}, {QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 0.75), QPen(Plot::red, 0.75)}, saveDir + "1DsXs" + to_string(SourceStride) + ".png");
    Plot1D::plot(toDegrees(Thetas), {polyfit(Thetas, ShiftsY, 3), ShiftsY, ShiftsYErrorsBot, ShiftsYErrorsTop}, "shifts profile Y", "solar latitude [deg]", "north-south image shift [px]", {"polyfit3", "average", "average - stdev", "average + stdev"}, {QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 0.75), QPen(Plot::red, 0.75)}, saveDir + "1DsYs" + to_string(SourceStride) + ".png");

    // flows ratio1
    Plot2D::plot(applyQuantile(FlowX, quanBot, quanTop), "diffrot flow X", "time [days]", "solar latitude [deg]", "west-east flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta), toDegrees(EndTheta), colRowRatio1, saveDir + "2DXm" + to_string(medianSize) + "r1s" + to_string(SourceStride) + ".png");
    Plot2D::plot(applyQuantile(FlowY, quanBot, quanTop), "diffrot flow Y", "time [days]", "solar latitude [deg]", "north-south flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta), toDegrees(EndTheta), colRowRatio1, saveDir + "2DYm" + to_string(medianSize) + "r1s" + to_string(SourceStride) + ".png");

    // flows ratio2
    Plot2D::plot(applyQuantile(FlowX, quanBot, quanTop), "diffrot flow X r", "time [days]", "solar latitude [deg]", "west-east flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta), toDegrees(EndTheta), colRowRatio2, saveDir + "2DXm" + to_string(medianSize) + "r2s" + to_string(SourceStride) + ".png");
    Plot2D::plot(applyQuantile(FlowY, quanBot, quanTop), "diffrot flow Y r", "time [days]", "solar latitude [deg]", "north-south flow speed [deg/day]", StartTime, EndTime, toDegrees(StartTheta), toDegrees(EndTheta), colRowRatio2, saveDir + "2DYm" + to_string(medianSize) + "r2s" + to_string(SourceStride) + ".png");

    LOG_INFO("Predic error = {}", GetError());
  }

  double GetError()
  {
    // used for optimizing - closest profile to literature profiles
    CalculateMainInterp1D();
    CalculatePredics();
    double error = 0;
    size_t ycount = OmegasX.size();
    const auto &mycurve = polyfit(Thetas, OmegasX, 2);
    const auto &targetcurve = 0.5 * (PredicXs[0] + PredicXs[1]);

    for (int y = 0; y < ycount; ++y)
      error += std::pow(mycurve[y] - targetcurve[y], 2);

    return sqrt(error / ycount);
  }

  static double Interpolate(const std::vector<double> &xs, const std::vector<double> &ys, double x)
  {
    if (xs.front() <= xs.back()) // ascending x
    {
      if (x <= xs.front())
        return ys.front();

      if (x >= xs.back())
        return ys.back();

      for (int i = 0; i < xs.size() - 1; ++i)
        if (xs[i] <= x && xs[i + 1] > x)
          return ys[i] + (x - xs[i]) / (xs[i + 1] - xs[i]) * (ys[i + 1] - ys[i]);
    }

    if (xs.front() > xs.back()) // descending x
    {
      if (x >= xs.front())
        return ys.front();

      if (x <= xs.back())
        return ys.back();

      for (int i = 0; i < xs.size() - 1; ++i)
        if (xs[i] > x && xs[i + 1] <= x)
          return ys[i + 1] + (x - xs[i + 1]) / (xs[i] - xs[i + 1]) * (ys[i] - ys[i + 1]);
    }

    LOG_FATAL("Interpolation fault");
    throw; // should never get here
  }

  static double Interpolate(const std::vector<std::vector<double>> &xs, const std::vector<std::vector<double>> &ys, double x)
  {
    const int ps = xs.size();
    double mean = 0;

    for (int p = 0; p < ps; ++p)
      mean += Interpolate(xs[p], ys[p], x);

    return mean / ps;
  }

  void SetData2D(const std::vector<std::vector<double>> &thetas2D, const std::vector<std::vector<double>> &omegasX, const std::vector<std::vector<double>> &omegasY, const std::vector<std::vector<double>> &shiftsX, const std::vector<std::vector<double>> &shiftsY)
  {
    SourceThetas = thetas2D;
    SourceShiftsX = shiftsX;
    SourceShiftsY = shiftsY;
    SourceOmegasX = omegasX;
    SourceOmegasY = omegasY;
    Data2DSet = true;
    UpdateCalculated();
  }

  void SetParams(int pics, int stride, std::string savepath)
  {
    SourcePics = pics;
    SourceStride = stride;
    saveDir = savepath;
    ParamsSet = true;
    UpdateCalculated();
  }

  auto GetVecs2D(std::vector<std::vector<double>> &sourceThetas, std::vector<std::vector<double>> &sourceShiftsX, std::vector<std::vector<double>> &sourceShiftsY, std::vector<std::vector<double>> &sourceOmegasX, std::vector<std::vector<double>> &sourceOmegasY) const
  {
    sourceThetas = SourceThetas;
    sourceShiftsX = SourceShiftsX;
    sourceShiftsY = SourceShiftsY;
    sourceOmegasX = SourceOmegasX;
    sourceOmegasY = SourceOmegasY;
  }

  auto GetParams(int &sourcePics, int &sourceStride) const
  {
    sourcePics = SourcePics;
    sourceStride = SourceStride;
  }

  void SetVecs2DRaw(const std::vector<std::vector<double>> &sourceThetas, const std::vector<std::vector<double>> &sourceShiftsX, const std::vector<std::vector<double>> &sourceShiftsY, const std::vector<std::vector<double>> &sourceOmegasX, const std::vector<std::vector<double>> &sourceOmegasY)
  {
    SourceThetas = sourceThetas;
    SourceShiftsX = sourceShiftsX;
    SourceShiftsY = sourceShiftsY;
    SourceOmegasX = sourceOmegasX;
    SourceOmegasY = sourceOmegasY;
  }

  void SetParamsRaw(int sourcePics, int sourceStride)
  {
    SourcePics = sourcePics;
    SourceStride = sourceStride;
  }

private:
  // source data
  int SourcePics;
  int SourceStride;
  std::vector<std::vector<double>> SourceThetas;
  std::vector<std::vector<double>> SourceShiftsX;
  std::vector<std::vector<double>> SourceShiftsY;
  std::vector<std::vector<double>> SourceOmegasX;
  std::vector<std::vector<double>> SourceOmegasY;

  // internal data
  std::vector<double> Thetas;
  std::vector<double> ShiftsX;
  std::vector<double> ShiftsY;
  std::vector<double> OmegasX;
  std::vector<double> OmegasY;
  Mat FlowX;
  Mat FlowY;
  bool Data2DSet = false;
  bool ParamsSet = false;
  static constexpr double colRowRatio1 = 2;
  static constexpr double colRowRatio2 = 1.5;
  static constexpr int predicCnt = 2;
  std::vector<std::vector<double>> PredicXs;
  std::vector<double> ShiftsXErrors;
  std::vector<double> ShiftsYErrors;
  std::vector<double> ShiftsXErrorsBot;
  std::vector<double> ShiftsXErrorsTop;
  std::vector<double> ShiftsYErrorsBot;
  std::vector<double> ShiftsYErrorsTop;
  std::vector<std::vector<double>> PredicXsNS;
  std::vector<double> ThetasNS;
  std::vector<double> OmegasXavgN;
  std::vector<double> OmegasXavgS;
  std::vector<double> OmegasYavgN;
  std::vector<double> OmegasYavgS;
  double StartTime;
  double EndTime;
  double StartTheta;
  double EndTheta;

  void CalculateMainInterp1D()
  {
    int ps = SourceThetas.size();
    int ys = SourceThetas[0].size();
    Thetas.resize(ys);
    ShiftsX.resize(ys);
    ShiftsY.resize(ys);
    OmegasX.resize(ys);
    OmegasY.resize(ys);

    // calculate theta range
    double thetaRange = Constants::Inf;
    for (int p = 0; p < ps; ++p)
      if (SourceThetas[p].front() < thetaRange)
        thetaRange = SourceThetas[p].front();
    double dth = 2. * thetaRange / (ys - 1);

    // interpolate shifts and omegas based on equidistant theta
    for (int y = 0; y < ys; ++y)
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
    int ps = SourceThetas.size();
    int ys = SourceThetas[0].size();

    FlowX = Mat::zeros(ys, ps, CV_32F);
    FlowY = Mat::zeros(ys, ps, CV_32F);

    for (int y = 0; y < ys; ++y)
    {
      for (int p = 0; p < ps; ++p)
      {
        FlowX.at<float>(y, ps - 1 - p) = Interpolate(SourceThetas[p], SourceOmegasX[p], Thetas[y]);
        FlowY.at<float>(y, ps - 1 - p) = Interpolate(SourceThetas[p], SourceOmegasY[p], Thetas[y]);
      }
    }
  }

  void UpdateCalculated() { calculated = Data2DSet && ParamsSet; }

  void CalculateMedianFilters(int medianSize)
  {
    if (medianSize)
    {
      for (int med = 3; med <= min(medianSize, 7); med += 2)
      {
        medianBlur(FlowX, FlowX, med);
        medianBlur(FlowY, FlowY, med);
      }
    }
  }

  void CalculateAxisLimits()
  {
    StartTime = 0;
    EndTime = (double)(SourcePics - 1) * SourceStride * 45 / 60 / 60 / 24;
    StartTheta = Thetas.front();
    EndTheta = Thetas.back();
  }

  void CalculatePredics()
  {
    PredicXs = zerovect2(predicCnt, Thetas.size(), 0.);
    for (int i = 0; i < Thetas.size(); i++)
    {
      PredicXs[0][i] = predictDiffrotProfile(Thetas[i], 14.296, -1.847, -2.615); // Derek A. Lamb (2017)
      PredicXs[1][i] = predictDiffrotProfile(Thetas[i], 14.192, -1.70, -2.36);   // Howard et al. (1983)
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

    int zeroidx = 0;
    for (int i = 0; i < Thetas.size() - 2; ++i)
    {
      if ((Thetas[i] > 0 && Thetas[i + 1] < 0) || Thetas[i] == 0)
      {
        zeroidx = i;
        LOG_INFO("Diffrot NS zeroidx = {}, ({:.2f} => {:.2f})", zeroidx, toDegrees(Thetas[i]), toDegrees(Thetas[i + 1]));
        break;
      }
    }

    // north hemisphere
    ThetasNS = std::vector<double>(Thetas.begin(), Thetas.begin() + zeroidx + 1);
    PredicXsNS[0] = std::vector<double>(PredicXs[0].begin(), PredicXs[0].begin() + zeroidx + 1);
    PredicXsNS[1] = std::vector<double>(PredicXs[1].begin(), PredicXs[1].begin() + zeroidx + 1);
    OmegasXavgN = std::vector<double>(OmegasX.begin(), OmegasX.begin() + zeroidx + 1);
    OmegasYavgN = std::vector<double>(OmegasY.begin(), OmegasY.begin() + zeroidx + 1);

    LOG_INFO("<NS> First (max) North theta = {:.2f}", toDegrees(ThetasNS.front()));
    LOG_INFO("<NS> Last (min) North theta = {:.2f}", toDegrees(ThetasNS.back()));

    // south hemisphere
    OmegasXavgS = std::vector<double>(OmegasX.begin() + zeroidx, OmegasX.begin() + 2 * zeroidx + 1);
    OmegasYavgS = std::vector<double>(OmegasY.begin() + zeroidx, OmegasY.begin() + 2 * zeroidx + 1);

    std::reverse(OmegasXavgS.begin(), OmegasXavgS.end());
    std::reverse(OmegasYavgS.begin(), OmegasYavgS.end());

    if (OmegasXavgN.size() != OmegasXavgS.size() || OmegasYavgN.size() != OmegasYavgS.size())
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
    LOG_NEWLINE;

    // XY both
    LogFitCoeffs("XcoeffsPoly2", polyfitCoeffs(toRadians(Thetas), OmegasX, 2));
    LogFitCoeffs("YcoeffsPoly3", polyfitCoeffs(toRadians(Thetas), OmegasY, 3));
    LogFitCoeffs("XcoeffsTrig", sin2sin4fitCoeffs(toRadians(Thetas), OmegasX));

    // X NS
    LogFitCoeffs("XcoeffsTrigN", sin2sin4fitCoeffs(toRadians(ThetasNS), OmegasXavgN));
    LogFitCoeffs("XcoeffsTrigS", sin2sin4fitCoeffs(toRadians(ThetasNS), OmegasXavgS));

    // Y NS
    LogFitCoeffs("YcoeffsTrigN", sin2sin4fitCoeffs(toRadians(ThetasNS), OmegasYavgN));
    LogFitCoeffs("YcoeffsTrigS", sin2sin4fitCoeffs(toRadians(ThetasNS), OmegasYavgS));
  }

  void LogFitCoeffs(const std::string &fitname, const std::vector<double> &coeffs)
  {
    for (int i = 0; i < coeffs.size(); i++)
      LOG_DEBUG("{} fit coefficient {} = {:.2f}", fitname, (char)('A' + i), coeffs[i]);

    LOG_NEWLINE;
  }
};
