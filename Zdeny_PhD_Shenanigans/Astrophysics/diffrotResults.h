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

    Reset();
    CalculateMedianFilters(medianSize);
    CalculateMagnitudeAndPhase();
    CalculateAxisLimits();
    CalculatePredics();
    CalculateErrors();
    CalculateNS();
    CalculateFitCoeffs();

    // ======================================================== PLOTS ========================================================

    // diffrot profiles
    Plot1D::plot(SourceThetasavg, std::vector<std::vector<double>>{polyfit(SourceThetasavg, SourceOmegasXavg, 2), SourceOmegasXavg, PredicXs[0], PredicXs[1]}, "diffrot profile X", "solar latitude [deg]", "west-east flow speed [deg/day]", std::vector<std::string>{"polyfit2", "average", "Derek A. Lamb (2017)", "Howard et al. (1983)"}, std::vector<QPen>{QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 2), QPen(Plot::red, 2)}, saveDir + "1DXs" + to_string(SourceStride) + ".png");
    Plot1D::plot(SourceThetasavg, std::vector<std::vector<double>>{polyfit(SourceThetasavg, SourceOmegasYavg, 3), SourceOmegasYavg}, "diffrot profile Y", "solar latitude [deg]", "north-south flow speed [deg/day]", std::vector<std::string>{"polyfit3", "average"}, std::vector<QPen>{QPen(Plot::black, 3), QPen(Plot::green, 1.5)}, saveDir + "1DYs" + to_string(SourceStride) + ".png"); // rgb(119, 136, 153)

    // diffrot profiles NS
    Plot1D::plot(ThetasNS, std::vector<std::vector<double>>{sin2sin4fit(toRadians(ThetasNS), OmegasXavgN), sin2sin4fit(toRadians(ThetasNS), OmegasXavgS), OmegasXavgN, OmegasXavgS, PredicXsNS[0], PredicXsNS[1]}, "diffrot profile NS X", "absolute solar latitude [deg]", "west-east flow speed [deg/day]", std::vector<std::string>{"trigfit North", "trigfit South", "average North", "average South", "Derek A. Lamb (2017)", "Howard et al. (1983)"}, std::vector<QPen>{QPen(Plot::blue, 3), QPen(Plot::red, 3), QPen(Plot::blue, 1.5), QPen(Plot::red, 1.5), QPen(Plot::green, 2), QPen(Plot::black, 2)}, saveDir + "1DNSXs" + to_string(SourceStride) + ".png");
    Plot1D::plot(ThetasNS, std::vector<std::vector<double>>{sin2sin4fit(toRadians(ThetasNS), OmegasYavgN), sin2sin4fit(toRadians(ThetasNS), OmegasYavgS), OmegasYavgN, OmegasYavgS}, "diffrot profile NS Y", "absolute solar latitude [deg]", "north-south flow speed [deg/day]", std::vector<std::string>{"trigfit North", "trigfit South", "average North", "average South"}, std::vector<QPen>{QPen(Plot::blue, 3), QPen(Plot::red, 3), QPen(Plot::blue, 1.5), QPen(Plot::red, 1.5)}, saveDir + "1DNSYs" + to_string(SourceStride) + ".png");

    // shifts profiles
    Plot1D::plot(SourceThetasavg, std::vector<std::vector<double>>{polyfit(SourceThetasavg, SourceShiftsXavg, 2), SourceShiftsXavg, ShiftsXErrorsBot, ShiftsXErrorsTop}, "shifts profile X", "solar latitude [deg]", "west-east image shift [px]", std::vector<std::string>{"polyfit2", "average", "average - stdev", "average + stdev"}, std::vector<QPen>{QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 0.75), QPen(Plot::red, 0.75)}, saveDir + "1DsXs" + to_string(SourceStride) + ".png");
    Plot1D::plot(SourceThetasavg, std::vector<std::vector<double>>{polyfit(SourceThetasavg, SourceShiftsYavg, 3), SourceShiftsYavg, ShiftsYErrorsBot, ShiftsYErrorsTop}, "shifts profile Y", "solar latitude [deg]", "north-south image shift [px]", std::vector<std::string>{"polyfit3", "average", "average - stdev", "average + stdev"}, std::vector<QPen>{QPen(Plot::black, 3), QPen(Plot::green, 1.5), QPen(Plot::blue, 0.75), QPen(Plot::red, 0.75)}, saveDir + "1DsYs" + to_string(SourceStride) + ".png");

    // flows ratio1
    Plot2D::plot(applyQuantile(FlowX, quanBot, quanTop), "diffrot flow X", "time [days]", "solar latitude [deg]", "west-east flow speed [deg/day]", StartTime, EndTime, StartTheta, EndTheta, colRowRatio1, saveDir + "2DXm" + to_string(medianSize) + "r1s" + to_string(SourceStride) + ".png");
    Plot2D::plot(applyQuantile(FlowY, quanBot, quanTop), "diffrot flow Y", "time [days]", "solar latitude [deg]", "north-south flow speed [deg/day]", StartTime, EndTime, StartTheta, EndTheta, colRowRatio1, saveDir + "2DYm" + to_string(medianSize) + "r1s" + to_string(SourceStride) + ".png");

    // flows ratio2
    Plot2D::plot(applyQuantile(FlowX, quanBot, quanTop), "diffrot flow X r", "time [days]", "solar latitude [deg]", "west-east flow speed [deg/day]", StartTime, EndTime, StartTheta, EndTheta, colRowRatio2, saveDir + "2DXm" + to_string(medianSize) + "r2s" + to_string(SourceStride) + ".png");
    Plot2D::plot(applyQuantile(FlowY, quanBot, quanTop), "diffrot flow Y r", "time [days]", "solar latitude [deg]", "north-south flow speed [deg/day]", StartTime, EndTime, StartTheta, EndTheta, colRowRatio2, saveDir + "2DYm" + to_string(medianSize) + "r2s" + to_string(SourceStride) + ".png");
  }

  double GetError()
  {
    // used for optimizing - closest profile to literature profiles
    CalculatePredics();
    double error = 0;
    size_t ycount = SourceOmegasXavg.size();
    size_t pcount = PredicXs.size();
    size_t y, p;

    for (y = 0; y < ycount; ++y)
      for (p = 0; p < pcount; ++p)
        error += std::pow(SourceOmegasXavg[y] - PredicXs[p][y], 2);

    return error / ycount / pcount;
  }

  void SetData1D(const std::vector<double> &thetasavg, const std::vector<double> &omegasXavg, const std::vector<double> &omegasYavg, const std::vector<double> &shiftsXavg, const std::vector<double> &shiftsYavg)
  {
    SourceThetasavg = (360. / Constants::TwoPi) * thetasavg;
    SourceOmegasXavg = omegasXavg;
    SourceOmegasYavg = omegasYavg;
    SourceShiftsXavg = shiftsXavg;
    SourceShiftsYavg = shiftsYavg;
    Data1DSet = true;
    UpdateCalculated();
  }

  void SetData2D(const std::vector<std::vector<double>> &image, const std::vector<std::vector<double>> &flowX, const std::vector<std::vector<double>> &flowY, const std::vector<std::vector<double>> &shiftsX, const std::vector<std::vector<double>> &shiftsY)
  {
    flip(matFromVector(image, true), SourceImage, 1);
    flip(matFromVector(flowX, true), SourceFlowX, 1);
    flip(matFromVector(flowY, true), SourceFlowY, 1);
    SourceShiftsX = shiftsX;
    SourceShiftsY = shiftsY;
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

  // fileIO store
  auto GetVecs1D(std::vector<double> &sourceThetasavg, std::vector<double> &sourceOmegasXavg, std::vector<double> &sourceOmegasYavg, std::vector<double> &sourceShiftsXavg, std::vector<double> &sourceShiftsYavg) const
  {
    sourceThetasavg = SourceThetasavg;
    sourceOmegasXavg = SourceOmegasXavg;
    sourceOmegasYavg = SourceOmegasYavg;
    sourceShiftsXavg = SourceShiftsXavg;
    sourceShiftsYavg = SourceShiftsYavg;
  }

  auto GetVecs2D(std::vector<std::vector<double>> &sourceShiftsX, std::vector<std::vector<double>> &sourceShiftsY) const
  {
    sourceShiftsX = SourceShiftsX;
    sourceShiftsY = SourceShiftsY;
  }

  auto GetMats(Mat &sourceImage, Mat &sourceFlowX, Mat &sourceFlowY) const
  {
    sourceImage = SourceImage;
    sourceFlowX = SourceFlowX;
    sourceFlowY = SourceFlowY;
  }

  auto GetParams(int &sourcePics, int &sourceStride) const
  {
    sourcePics = SourcePics;
    sourceStride = SourceStride;
  }

  // fileIO load
  void SetVecs1DRaw(const std::vector<double> &sourceThetasavg, const std::vector<double> &sourceOmegasXavg, const std::vector<double> &sourceOmegasYavg, const std::vector<double> &sourceShiftsXavg, const std::vector<double> &sourceShiftsYavg)
  {
    SourceThetasavg = sourceThetasavg;
    SourceOmegasXavg = sourceOmegasXavg;
    SourceOmegasYavg = sourceOmegasYavg;
    SourceShiftsXavg = sourceShiftsXavg;
    SourceShiftsYavg = sourceShiftsYavg;
  }

  void SetVecs2DRaw(const std::vector<std::vector<double>> &sourceShiftsX, const std::vector<std::vector<double>> &sourceShiftsY)
  {
    SourceShiftsX = sourceShiftsX;
    SourceShiftsY = sourceShiftsY;
  }

  void SetMatsRaw(const Mat &sourceImage, const Mat &sourceFlowX, const Mat &sourceFlowY)
  {
    SourceImage = sourceImage;
    SourceFlowX = sourceFlowX;
    SourceFlowY = sourceFlowY;
  }

  void SetParamsRaw(int sourcePics, int sourceStride)
  {
    SourcePics = sourcePics;
    SourceStride = sourceStride;
  }

private:
  // source data
  Mat SourceImage;
  Mat SourceFlowX;
  Mat SourceFlowY;
  int SourcePics;
  int SourceStride;
  std::vector<std::vector<double>> SourceShiftsX;
  std::vector<std::vector<double>> SourceShiftsY;
  std::vector<double> SourceThetasavg;
  std::vector<double> SourceOmegasXavg;
  std::vector<double> SourceOmegasYavg;
  std::vector<double> SourceShiftsXavg;
  std::vector<double> SourceShiftsYavg;

  // internal data
  bool Data1DSet = false;
  bool Data2DSet = false;
  bool ParamsSet = false;
  Mat FlowX;
  Mat FlowY;
  Mat FlowM;
  Mat FlowP;
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

  void Reset()
  {
    FlowX = SourceFlowX.clone();
    FlowY = SourceFlowY.clone();
    resize(FlowY, FlowY, FlowX.size());
  }

  void UpdateCalculated() { calculated = Data1DSet && Data2DSet && ParamsSet; }

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

  void CalculateMagnitudeAndPhase()
  {
    magnitude(FlowX, FlowY, FlowM);
    phase(FlowX, FlowY, FlowP, true);
  }

  void CalculateAxisLimits()
  {
    StartTime = 0;
    EndTime = (double)(SourcePics - 1) * SourceStride * 45 / 60 / 60 / 24;
    StartTheta = SourceThetasavg.front();
    EndTheta = SourceThetasavg.back();
  }

  void CalculatePredics()
  {
    PredicXs = zerovect2(predicCnt, SourceThetasavg.size(), 0.);
    for (int i = 0; i < SourceThetasavg.size(); i++)
    {
      PredicXs[0][i] = predictDiffrotProfile(toRadians(SourceThetasavg[i]), 14.296, -1.847, -2.615); // Derek A. Lamb (2017)
      PredicXs[1][i] = predictDiffrotProfile(toRadians(SourceThetasavg[i]), 14.192, -1.70, -2.36);   // Howard et al. (1983)
      // etc...
    }
  }

  void CalculateErrors()
  {
    ShiftsXErrors = getStandardDeviationsVertical(SourceShiftsX);
    ShiftsYErrors = getStandardDeviationsVertical(SourceShiftsY);
    ShiftsXErrorsBot = SourceShiftsXavg - ShiftsXErrors;
    ShiftsYErrorsBot = SourceShiftsYavg - ShiftsYErrors;
    ShiftsXErrorsTop = SourceShiftsXavg + ShiftsXErrors;
    ShiftsYErrorsTop = SourceShiftsYavg + ShiftsYErrors;
  }

  double Interpolate(const std::vector<double> &xs, const std::vector<double> &ys, double x)
  {
    int l, u;
    double r;

    for (int i = 0; i < xs.size() - 2; ++i)
    {
      if (xs[i] < x && xs[i + 1] > x)
      {
        l = i;
        u = i + 1;
        r = (x - xs[l]) / (xs[u] - xs[l]);
        break;
      }

      if (xs[i] > x && xs[i + 1] < x)
      {
        u = i;
        l = i + 1;
        r = (x - xs[l]) / (xs[u] - xs[l]);
        break;
      }
    }

    return ys[l] + r * (ys[u] - ys[l]);
  }

  void CalculateNS()
  {
    PredicXsNS.resize(predicCnt);

    int zeroidx = 0;
    for (int i = 0; i < SourceThetasavg.size() - 2; ++i)
    {
      if ((SourceThetasavg[i] > 0 && SourceThetasavg[i + 1] < 0) || SourceThetasavg[i] == 0)
      {
        zeroidx = i;
        LOG_DEBUG("Diffrot NS zeroidx = {}, ({:.2f} => {:.2f})", zeroidx, SourceThetasavg[i], SourceThetasavg[i + 1]);
        break;
      }
    }

    // north hemisphere
    ThetasNS = std::vector<double>(SourceThetasavg.begin(), SourceThetasavg.begin() + zeroidx + 1);
    PredicXsNS[0] = std::vector<double>(PredicXs[0].begin(), PredicXs[0].begin() + zeroidx + 1);
    PredicXsNS[1] = std::vector<double>(PredicXs[1].begin(), PredicXs[1].begin() + zeroidx + 1);
    OmegasXavgN = std::vector<double>(SourceOmegasXavg.begin(), SourceOmegasXavg.begin() + zeroidx + 1);
    OmegasYavgN = std::vector<double>(SourceOmegasYavg.begin(), SourceOmegasYavg.begin() + zeroidx + 1);

    LOG_DEBUG("<NS> First (max) North theta = {:.2f}", ThetasNS.front());
    LOG_DEBUG("<NS> Last (min) North theta = {:.2f}", ThetasNS.back());

    // south hemisphere
    OmegasXavgS.resize(OmegasXavgN.size());
    OmegasYavgS.resize(OmegasXavgS.size());

    for (int i = 0; i < OmegasXavgN.size(); i++)
    {
      OmegasXavgS[i] = Interpolate(SourceThetasavg, SourceOmegasXavg, -ThetasNS[i]);
      OmegasYavgS[i] = Interpolate(SourceThetasavg, SourceOmegasYavg, -ThetasNS[i]);
    }

    if (OmegasXavgN.size() != OmegasXavgS.size() || OmegasYavgN.size() != OmegasYavgS.size())
    {
      LOG_ERROR("<NS> NS sizes mismatch: {} != {} & {} != {}", OmegasXavgN.size(), OmegasXavgS.size(), OmegasYavgN.size(), OmegasYavgS.size());
      throw;
    }

    LOG_DEBUG("<NS> NS equator values X: {:.2f} = {:.2f}", OmegasXavgN.back(), OmegasXavgS.back());
    LOG_DEBUG("<NS> NS equator values Y: {:.2f} = {:.2f}", OmegasYavgN.back(), OmegasYavgS.back());
  }

  void CalculateFitCoeffs()
  {
    LOG_NEWLINE;

    // XY both
    LogFitCoeffs("XcoeffsPoly2", polyfitCoeffs(toRadians(SourceThetasavg), SourceOmegasXavg, 2));
    LogFitCoeffs("YcoeffsPoly3", polyfitCoeffs(toRadians(SourceThetasavg), SourceOmegasYavg, 3));
    LogFitCoeffs("XcoeffsTrig", sin2sin4fitCoeffs(toRadians(SourceThetasavg), SourceOmegasXavg));

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
    {
      LOG_INFO("{} fit coefficient {} = {:.2f}", fitname, (char)('A' + i), coeffs[i]);
    }
    LOG_NEWLINE;
  }
};
