#pragma once
#include "IPC/IterativePhaseCorrelation.h"
#include "Fit/Polyfit.h"

class DifferentialRotation
{
public:
  DifferentialRotation(i32 xsize_ = 2500, i32 ysize_ = 851, i32 idstep_ = 1, i32 idstride_ = 25, i32 yfov_ = 3400, i32 cadence_ = 45)
      : xsize(xsize_), ysize(ysize_), idstep(idstep_), idstride(idstride_), yfov(yfov_), cadence(cadence_)
  {
  }

  struct DifferentialRotationData
  {
    DifferentialRotationData(i32 xsize, i32 ysize)
    {
      thetas = cv::Mat::zeros(ysize, xsize, CV_32F);
      shiftsx = cv::Mat::zeros(ysize, xsize, CV_32F);
      shiftsy = cv::Mat::zeros(ysize, xsize, CV_32F);
      omegasx = cv::Mat::zeros(ysize, xsize, CV_32F);
      omegasy = cv::Mat::zeros(ysize, xsize, CV_32F);
      fshiftx = std::vector<f64>(xsize, 0.);
      fshifty = std::vector<f64>(xsize, 0.);
      theta0s = std::vector<f64>(xsize, 0.);
      Rs = std::vector<f64>(xsize, 0.);
    }

    cv::Mat thetas, shiftsx, shiftsy, omegasx, omegasy;
    std::vector<f64> fshiftx, fshifty, theta0s, Rs;
  };

  template <bool Managed = false> // executed automatically by some logic (e.g. optimization algorithm) instead of manually
  DifferentialRotationData Calculate(const IterativePhaseCorrelation& ipc, const std::string& dataPath, i32 idstart) const
  try
  {
    LOG_FUNCTION_IF(not Managed, "DifferentialRotation::Calculate");
    DifferentialRotationData data(xsize, ysize);
    std::atomic<i32> progress = -1;
    const auto ystep = yfov / (ysize - 1);
    const auto tstep = idstep * cadence;
    const auto wxsize = ipc.GetCols();
    const auto wysize = ipc.GetRows();
    const auto shiftxmin = 0.01 * idstep;
    const auto shiftxmax = 0.4 * idstep;
    const auto shiftymax = 0.08;
    const auto ids = GenerateIds(idstart);

#pragma omp parallel for if (not Managed)
    for (i32 x = 0; x < xsize; ++x)
      try
      {
        const auto logprogress = static_cast<f64>(progress.fetch_add(1, std::memory_order_relaxed)) + 1.;
        const auto& [id1, id2] = ids[x];
        const std::string path1 = fmt::format("{}/{}.png", dataPath, id1);
        const std::string path2 = fmt::format("{}/{}.png", dataPath, id2);
        if (std::filesystem::exists(path1) and std::filesystem::exists(path2))
          [[likely]]
          {
            if constexpr (not Managed)
              LOG_DEBUG("[{:>3.0f}%: {:>4} / {:>4}] Calculating diffrot profile {} - {} ...", logprogress / (xsize - 1) * 100, logprogress + 1, xsize, id1, id2);
          }
        else
          [[unlikely]]
          {
            if constexpr (not Managed)
              LOG_WARNING("[{:>3.0f}%: {:>4} / {:>4}] Could not load images {} - {}, skipping ...", logprogress / (xsize - 1) * 100, logprogress + 1, xsize, id1, id2);
            continue;
          }

        const auto image1 = cv::imread(path1, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
        const auto image2 = cv::imread(path2, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
        const auto header1 = GetHeader(fmt::format("{}/{}.json", dataPath, id1));
        const auto header2 = GetHeader(fmt::format("{}/{}.json", dataPath, id2));
        const auto theta0 = (header1.theta0 + header2.theta0) / 2;
        const auto R = (header1.R + header2.R) / 2;
        const auto xindex = xsize - 1 - x;
        data.fshiftx[xindex] = header2.xcenter - header1.xcenter;
        data.fshifty[xindex] = header2.ycenter - header1.ycenter;
        data.theta0s[xindex] = theta0;
        data.Rs[xindex] = R;

        for (i32 y = 0; y < ysize; ++y)
        {
          const auto yshift = ystep * (y - ysize / 2);
          const auto theta = std::asin((f64)(-yshift) / R) + theta0;
          const auto omegaxpred = GetPredictedOmega(theta, 14.296, -1.847, -2.615);
          auto crop1 = roicrop(image1, header1.xcenter, header1.ycenter + yshift, wxsize, wysize);
          auto crop2 = roicrop(image2, header2.xcenter, header2.ycenter + yshift, wxsize, wysize);
          const auto shift = ipc.Calculate(std::move(crop1), std::move(crop2));
          const auto shiftx = std::clamp(shift.x, shiftxmin, shiftxmax);
          const auto shifty = std::clamp(shift.y, -shiftymax, shiftymax);
          const auto omegax = std::clamp(std::asin(shiftx / (R * std::cos(theta))) / tstep * Constants::RadPerSecToDegPerDay, 0.7 * omegaxpred, 1.3 * omegaxpred);
          const auto omegay = (theta - std::asin(std::sin(theta) - shifty / R)) / tstep * Constants::RadPerSecToDegPerDay;

          data.thetas.at<f32>(y, xindex) = theta;
          data.shiftsx.at<f32>(y, xindex) = shiftx;
          data.shiftsy.at<f32>(y, xindex) = shifty;
          data.omegasx.at<f32>(y, xindex) = omegax;
          data.omegasy.at<f32>(y, xindex) = omegay;
        }
      }
      catch (const std::exception& e)
      {
        if constexpr (not Managed)
          LOG_WARNING("DifferentialRotation::Calculate error: {} - skipping ...", e.what());
        continue;
      }

    FixMissingData(data);

    if constexpr (not Managed)
      Save(data, ipc, dataPath);

    if constexpr (not Managed)
      Plot(data, dataPath, idstart);

    return data;
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("DifferentialRotation::Calculate error: {}", e.what());
    return DifferentialRotationData(0, 0);
  }

  void LoadAndShow(const std::string& path, const std::string& dataPath, i32 idstart)
  try
  {
    LOG_FUNCTION("DifferentialRotation::LoadAndShow");
    const auto data = Load(path);
    Plot(data, dataPath, idstart);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("DifferentialRotation::LoadAndShow error: {}", e.what());
  }

  void Optimize(IterativePhaseCorrelation& ipc, const std::string& dataPath, i32 idstart, i32 xsizeopt, i32 ysizeopt, i32 popsize) const
  {
    const auto f = [&](const IterativePhaseCorrelation& _ipc) {
      DifferentialRotation diffrot(xsizeopt, ysizeopt, idstep, idstride, yfov, cadence);
      const auto rawdata = diffrot.Calculate<true>(_ipc, dataPath, idstart);
      const auto [data, thetas] = PostProcessData(rawdata);
      const auto predfit = GetPredictedOmegas(thetas, 14.296, -1.847, -2.615);
      const auto omegasx = GetXAverage(data.omegasx);
      const auto omegasxfit = polyfit(thetas, omegasx, 2);

      f32 ret = 0;
      for (usize i = 0; i < omegasxfit.size(); ++i)
      {
        ret += std::pow(omegasxfit[i] - predfit[i], 2);       // ipc fit - pred fit diff
        ret += 0.5 * std::pow(omegasx[i] - omegasxfit[i], 2); // ipc - fit diff
      }
      return ret / omegasxfit.size();
    };

    DifferentialRotation diffrot(xsizeopt, ysizeopt, idstep, idstride, yfov, cadence);
    const auto [dataBefore, thetas] = PostProcessData(diffrot.Calculate<true>(ipc, dataPath, idstart));
    ipc.Optimize(f, popsize);
    SaveOptimizedParameters(ipc, dataPath, xsizeopt, ysizeopt, popsize);
    const auto [dataAfter, _] = PostProcessData(diffrot.Calculate<true>(ipc, dataPath, idstart));

    Plot1D::Set("Diffrot opt");
    Plot1D::SetXlabel("latitude [deg]");
    Plot1D::SetYlabel("average omega x [deg/day]");
    Plot1D::SetYnames({"avgomega x", "avgomega x fit", "avgomega x opt", "avgomega x opt fit", "Derek A. Lamb (2017)"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot(Constants::Rad * thetas, {GetXAverage(dataBefore.omegasx), polyfit(thetas, GetXAverage(dataBefore.omegasx), 2), GetXAverage(dataAfter.omegasx),
                                              polyfit(thetas, GetXAverage(dataAfter.omegasx), 2), GetPredictedOmegas(thetas, 14.296, -1.847, -2.615)});
  }

  static std::tuple<DifferentialRotationData, std::vector<f64>> PostProcessData(const DifferentialRotationData& rawdata)
  {
    const auto thetas = GetInterpolatedThetas(rawdata.thetas);
    DifferentialRotationData data = rawdata;

    // apply median blur
    static constexpr i32 medsize = 3;
    cv::medianBlur(data.shiftsx, data.shiftsx, medsize);
    cv::medianBlur(data.shiftsy, data.shiftsy, medsize);
    cv::medianBlur(data.omegasx, data.omegasx, medsize);
    cv::medianBlur(data.omegasy, data.omegasy, medsize);

    // apply theta-interpolation
    Interpolate(data.shiftsx, data.thetas, thetas);
    Interpolate(data.shiftsy, data.thetas, thetas);
    Interpolate(data.omegasx, data.thetas, thetas);
    Interpolate(data.omegasy, data.thetas, thetas);

    return {data, thetas};
  }

  static void PlotMeridianCurve(const DifferentialRotationData& data, const std::vector<f64>& thetas, const std::string& dataPath, i32 idstart, f64 timestep)
  {
    const auto imagegrs = cv::imread(fmt::format("{}/{}.png", dataPath, idstart), cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
    const auto header = GetHeader(fmt::format("{}/{}.json", dataPath, idstart));
    const auto R = header.R;              // [px]
    const auto theta0 = header.theta0;    // [rad]
    const auto fxcenter = header.xcenter; // [px]
    const auto fycenter = header.ycenter; // [px]
    // const auto omegasx = GetXAverage(data.omegasx);                          // [deg/day]
    const auto omegasx = GetPredictedOmegas(thetas, 14.296, -1.847, -2.615); // [deg/day]
    std::vector<cv::Point2d> mcpts(thetas.size());                           // [px,px]

    for (usize y = 0; y < thetas.size(); ++y)
    {
      const f64 mcx = fxcenter + R * std::cos(thetas[y]) * std::sin(omegasx[y] * timestep / Constants::Rad);
      const f64 mcy = fycenter - R * std::sin(thetas[y] - theta0);
      mcpts[y] = cv::Point2d(mcx, mcy);
    }

    cv::Mat image;
    cv::cvtColor(imagegrs, image, cv::COLOR_GRAY2RGB);
    const auto thickness = 13;
    const auto color = 65535. / 255 * cv::Scalar(0., 255., 255.);
    for (usize y = 0; y < mcpts.size() - 1; ++y)
    {
      const auto pt1 = mcpts[y];
      const auto pt2 = mcpts[y + 1];
      cv::line(image, pt1, pt2, color, thickness, cv::LINE_AA);
    }

    showimg(image, "meridian curve", false, 0, 1, 1200);
    saveimg(fmt::format("{}/meridian_curve.png", dataPath), image);

    Plot1D::Set("meridian curve omegasx");
    Plot1D::SetXlabel("latitude [deg]");
    Plot1D::SetYlabel("omega x [deg/day]");
    Plot1D::SetYnames({"Derek A. Lamb (2017)", "Howard et al. (1983)"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::SetSavePath(fmt::format("{}/meridian_curve_omega.png", dataPath));
    Plot1D::Plot(Constants::Rad * thetas, {GetPredictedOmegas(thetas, 14.296, -1.847, -2.615), GetPredictedOmegas(thetas, 14.192, -1.70, -2.36)});
  }

private:
  struct ImageHeader
  {
    f64 xcenter; // [px]
    f64 ycenter; // [px]
    f64 theta0;  // [rad]
    f64 R;       // [px]
  };

  static ImageHeader GetHeader(const std::string& path)
  {
    std::ifstream file(path);
    json::json j;
    file >> j;

    ImageHeader header;
    header.xcenter = (j["NAXIS1"].get<f64>()) - (j["CRPIX1"].get<f64>()); // x is flipped, 4095 - fits index from 1
    header.ycenter = j["CRPIX2"].get<f64>() - 1;                          // fits index from 1
    header.theta0 = j["CRLT_OBS"].get<f64>() / Constants::Rad;            // convert from deg to rad
    header.R = j["RSUN_OBS"].get<f64>() / j["CDELT1"].get<f64>();         // arcsec / arcsec per pixel
    return header;
  }

  std::vector<std::pair<i32, i32>> GenerateIds(i32 idstart) const
  {
    std::vector<std::pair<i32, i32>> ids(xsize);
    i32 id = idstart;
    for (i32 x = 0; x < xsize; ++x)
    {
      ids[x] = {id, id + idstep};
      id += idstride != 0 ? idstride : idstep;
    }
    return ids;
  }

  static void FixMissingData(DifferentialRotationData& data)
  {
    // fix missing data by interpolation
    for (i32 x = 0; x < data.thetas.cols; ++x)
    {
      if (data.thetas.at<f32>(0, x) != 0.0f) // no need to fix, data not missing
        continue;

      // find first non-missing previous data
      auto xindex1 = std::max(x - 1, 0);

      // find first non-missing next data
      auto xindex2 = std::min(x + 1, data.thetas.cols - 1);
      while (data.thetas.at<f32>(0, xindex2) == 0.0f and xindex2 < data.thetas.cols - 1)
        ++xindex2;

      const f64 t = (static_cast<f64>(x) - xindex1) / (xindex2 - xindex1);
      LOG_DEBUG("Fixing missing data: {} < x({}) < {}, t: {:.2f} ...", xindex1, x, xindex2, t);

      for (i32 y = 0; y < data.thetas.rows; ++y)
      {
        data.fshiftx[x] = std::lerp(data.fshiftx[xindex1], data.fshiftx[xindex2], t);
        data.fshifty[x] = std::lerp(data.fshifty[xindex1], data.fshifty[xindex2], t);
        data.theta0s[x] = std::lerp(data.theta0s[xindex1], data.theta0s[xindex2], t);
        data.Rs[x] = std::lerp(data.Rs[xindex1], data.Rs[xindex2], t);
        data.thetas.at<f32>(y, x) = std::lerp(data.thetas.at<f32>(y, xindex1), data.thetas.at<f32>(y, xindex2), static_cast<f32>(t));
        data.shiftsx.at<f32>(y, x) = std::lerp(data.shiftsx.at<f32>(y, xindex1), data.shiftsx.at<f32>(y, xindex2), static_cast<f32>(t));
        data.shiftsy.at<f32>(y, x) = std::lerp(data.shiftsy.at<f32>(y, xindex1), data.shiftsy.at<f32>(y, xindex2), static_cast<f32>(t));
        data.omegasx.at<f32>(y, x) = std::lerp(data.omegasx.at<f32>(y, xindex1), data.omegasx.at<f32>(y, xindex2), static_cast<f32>(t));
        data.omegasy.at<f32>(y, x) = std::lerp(data.omegasy.at<f32>(y, xindex1), data.omegasy.at<f32>(y, xindex2), static_cast<f32>(t));
      }
    }
  }

  static std::vector<f64> GetInterpolatedThetas(const cv::Mat& thetas)
  {
    f64 ithetamin = -std::numeric_limits<f64>::infinity(); // highest S latitude
    f64 ithetamax = std::numeric_limits<f64>::infinity();  // lowest N latitude
    for (i32 x = 0; x < thetas.cols; ++x)
    {
      const auto minval = thetas.at<f32>(thetas.rows - 1, x);
      const auto maxval = thetas.at<f32>(0, x);
      if (minval == 0 and maxval == 0)
        [[unlikely]] continue;
      ithetamin = std::max(ithetamin, static_cast<f64>(minval));
      ithetamax = std::min(ithetamax, static_cast<f64>(maxval));
    }

    if (ithetamin == -std::numeric_limits<f64>::infinity() or ithetamax == std::numeric_limits<f64>::infinity())
      throw std::runtime_error("Failed to calculate interpolated thetas");

    // LOG_DEBUG("InterpolatedTheta min / max: {:.1f} / {:.1f}", ithetamin * Constants::Rad, ithetamax * Constants::Rad);

    std::vector<f64> ithetas(thetas.rows);
    f64 ithetastep = (ithetamax - ithetamin) / (thetas.rows - 1);
    for (i32 y = 0; y < thetas.rows; ++y)
      ithetas[y] = ithetamax - ithetastep * y;
    return ithetas;
  }

  static std::vector<f64> GetTimesInDays(i32 tstep, i32 tstride, i32 xsize)
  {
    std::vector<f64> times(xsize);
    f64 time = 0;
    for (i32 i = 0; i < xsize; ++i)
    {
      times[i] = time / 60 / 60 / 24;
      time += tstride != 0 ? tstride : tstep;
    }
    return times;
  }

  static i32 FindFirstLowerIndex(const cv::Mat& thetas, i32 x, f64 itheta)
  {
    for (i32 y = 1; y < thetas.rows; ++y)
      if (itheta >= thetas.at<f32>(y, x))
        return y - 1;

    return thetas.rows - 2;
  }

  static void Interpolate(cv::Mat& vals, const cv::Mat& thetas, const std::vector<f64>& ithetas)
  {
    cv::Mat ivals = cv::Mat::zeros(vals.rows, vals.cols, CV_32F);
    for (i32 x = 0; x < vals.cols; ++x)
    {
      for (i32 y = 0; y < vals.rows; ++y)
      {
        const auto theta = ithetas[y];
        const auto iL = FindFirstLowerIndex(thetas, x, theta);
        const auto iH = iL + 1;
        const auto thetaL = thetas.at<f32>(iL, x);
        const auto thetaH = thetas.at<f32>(iH, x);
        const f64 valL = vals.at<f32>(iL, x);
        const f64 valH = vals.at<f32>(iH, x);
        const f64 t = (theta - thetaL) / (thetaH - thetaL);
        ivals.at<f32>(y, x) = std::lerp(valL, valH, t);
      }
    }
    vals = ivals;
  }

  static std::vector<f64> GetXAverage(const cv::Mat& vals)
  {
    std::vector<f64> avgs(vals.rows, 0.);

    for (i32 y = 0; y < vals.rows; ++y)
      for (i32 x = 0; x < vals.cols; ++x)
        avgs[y] += vals.at<f32>(y, x);

    for (i32 y = 0; y < vals.rows; ++y)
      avgs[y] /= vals.cols;

    return avgs;
  }

  static f64 GetPredictedOmega(f64 theta, f64 A, f64 B, f64 C) { return A + B * std::pow(std::sin(theta), 2) + C * std::pow(std::sin(theta), 4); }

  static std::vector<f64> GetPredictedOmegas(const std::vector<f64>& thetas, f64 A, f64 B, f64 C)
  {
    std::vector<f64> omegas(thetas.size(), 0.);
    for (usize i = 0; i < thetas.size(); ++i)
      omegas[i] = GetPredictedOmega(thetas[i], A, B, C);
    return omegas;
  }

  void Plot(const DifferentialRotationData& rawdata, const std::string& dataPath, i32 idstart) const
  {
    LOG_FUNCTION("DifferentialRotation::Plot");
    const auto [data, thetas] = PostProcessData(rawdata);
    const auto times = GetTimesInDays(idstep * cadence, idstride * cadence, xsize);

    PlotMeridianCurve(data, thetas, dataPath, idstart, 27);

    // fits params
    Plot1D::Set("fits params");
    Plot1D::SetXlabel("time [days]");
    Plot1D::SetYlabel("fits shift [px]");
    Plot1D::SetY2label("theta0 [deg]");
    Plot1D::SetYnames({"fits shift x", "fits shift y"});
    Plot1D::SetY2names({"theta0"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot(times, {data.fshiftx, data.fshifty}, {Constants::Rad * data.theta0s});

    // average shifts x / y
    Plot1D::Set("avgshiftsx");
    Plot1D::SetXlabel("latitude [deg]");
    Plot1D::SetYlabel("average shift x [px]");
    Plot1D::SetY2label("average shift y [px]");
    Plot1D::SetYnames({"avgshift x"});
    Plot1D::SetY2names({"avgshift y"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot(Constants::Rad * thetas, {GetXAverage(data.shiftsx)}, {GetXAverage(data.shiftsy)});

    // average omegas x
    Plot1D::Set("avgomegasx");
    Plot1D::SetXlabel("latitude [deg]");
    Plot1D::SetYlabel("average omega x [deg/day]");
    Plot1D::SetYnames({"avgomega x", "avgomega x fit", "Derek A. Lamb (2017)", "Howard et al. (1983)"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot(Constants::Rad * thetas,
        {GetXAverage(data.omegasx), polyfit(thetas, GetXAverage(data.omegasx), 2), GetPredictedOmegas(thetas, 14.296, -1.847, -2.615), GetPredictedOmegas(thetas, 14.192, -1.70, -2.36)});

    // shifts x
    Plot2D::Set("shiftsx");
    Plot2D::SetColRowRatio(1.5);
    Plot2D::ShowAxisLabels(true);
    Plot2D::SetXmin(times.front());
    Plot2D::SetXmax(times.back());
    Plot2D::SetYmin(thetas.back() * Constants::Rad);
    Plot2D::SetYmax(thetas.front() * Constants::Rad);
    Plot2D::SetXlabel("time [days]");
    Plot2D::SetYlabel("latitude [deg]");
    Plot2D::SetZlabel("shifts x [px]");
    Plot2D::Plot(data.shiftsx);

    // omegas x
    Plot2D::Set("omegasx");
    Plot2D::SetColRowRatio(1.5);
    Plot2D::ShowAxisLabels(true);
    Plot2D::SetXmin(times.front());
    Plot2D::SetXmax(times.back());
    Plot2D::SetYmin(thetas.back() * Constants::Rad);
    Plot2D::SetYmax(thetas.front() * Constants::Rad);
    Plot2D::SetXlabel("time [days]");
    Plot2D::SetYlabel("latitude [deg]");
    Plot2D::SetZlabel("omegas x [px]");
    Plot2D::Plot(data.omegasx);

    // shifts y
    Plot2D::Set("shiftsy");
    Plot2D::SetColRowRatio(1.5);
    Plot2D::ShowAxisLabels(true);
    Plot2D::SetXmin(times.front());
    Plot2D::SetXmax(times.back());
    Plot2D::SetYmin(thetas.back() * Constants::Rad);
    Plot2D::SetYmax(thetas.front() * Constants::Rad);
    Plot2D::SetXlabel("time [days]");
    Plot2D::SetYlabel("latitude [deg]");
    Plot2D::SetZlabel("shifts y [px]");
    Plot2D::Plot(data.shiftsy);

    // omegas y
    Plot2D::Set("omegasy");
    Plot2D::SetColRowRatio(1.5);
    Plot2D::ShowAxisLabels(true);
    Plot2D::SetXmin(times.front());
    Plot2D::SetXmax(times.back());
    Plot2D::SetYmin(thetas.back() * Constants::Rad);
    Plot2D::SetYmax(thetas.front() * Constants::Rad);
    Plot2D::SetXlabel("time [days]");
    Plot2D::SetYlabel("latitude [deg]");
    Plot2D::SetZlabel("omegas y [px]");
    Plot2D::Plot(data.omegasy);
  }

  void Save(const DifferentialRotationData& data, const IterativePhaseCorrelation& ipc, const std::string& dataPath) const
  {
    LOG_FUNCTION("DifferentialRotation::Save");

    std::string path = fmt::format("{}/diffrot.json", dataPath);
    LOG_DEBUG("Saving differential rotation results to {} ...", std::filesystem::weakly_canonical(path));
    cv::FileStorage file(path, cv::FileStorage::WRITE);

    // diffrot params
    file << "xsize" << xsize;
    file << "ysize" << ysize;
    file << "idstep" << idstep;
    file << "idstride" << idstride;
    file << "yfov" << yfov;
    file << "cadence" << cadence;
    // ipc params
    file << "wxsize" << ipc.GetCols();
    file << "wysize" << ipc.GetRows();
    file << "bandpassL" << ipc.GetBandpassL();
    file << "bandpassH" << ipc.GetBandpassH();
    file << "L2size" << ipc.GetL2size();
    file << "L1ratio" << ipc.GetL1ratio();
    file << "UpsampleCoeff" << ipc.GetUpsampleCoeff();
    file << "BandpassType" << ipc.BandpassType2String(ipc.GetBandpassType(), ipc.GetBandpassL(), ipc.GetBandpassH());
    file << "WindowType" << ipc.WindowType2String(ipc.GetWindowType());
    file << "InterpolationType" << ipc.InterpolationType2String(ipc.GetInterpolationType());
    // diffrot data
    file << "thetas" << data.thetas;
    file << "shiftsx" << data.shiftsx;
    file << "shiftsy" << data.shiftsy;
    file << "omegasx" << data.omegasx;
    file << "omegasy" << data.omegasy;
    file << "fshiftx" << data.fshiftx;
    file << "fshifty" << data.fshifty;
    file << "theta0s" << data.theta0s;
    file << "Rs" << data.Rs;
  }

  void SaveOptimizedParameters(const IterativePhaseCorrelation& ipc, const std::string& dataPath, i32 xsizeopt, i32 ysizeopt, i32 popsize) const
  {
    LOG_FUNCTION("DifferentialRotation::SaveOptimizedParameters");
    std::string path = fmt::format("{}/diffrot_ipcopt.json", dataPath);
    LOG_DEBUG("Saving differential rotation IPC optimization results to {} ...", std::filesystem::weakly_canonical(path));

    cv::FileStorage file(path, cv::FileStorage::WRITE);
    // diffrot opt params
    file << "xsizeopt" << xsizeopt;
    file << "ysizeopt" << ysizeopt;
    file << "popsize" << popsize;
    // ipc params
    file << "wxsize" << ipc.GetCols();
    file << "wysize" << ipc.GetRows();
    file << "bandpassL" << ipc.GetBandpassL();
    file << "bandpassH" << ipc.GetBandpassH();
    file << "L2size" << ipc.GetL2size();
    file << "L1ratio" << ipc.GetL1ratio();
    file << "UpsampleCoeff" << ipc.GetUpsampleCoeff();
    file << "BandpassType" << ipc.BandpassType2String(ipc.GetBandpassType(), ipc.GetBandpassL(), ipc.GetBandpassH());
    file << "WindowType" << ipc.WindowType2String(ipc.GetWindowType());
    file << "InterpolationType" << ipc.InterpolationType2String(ipc.GetInterpolationType());
  }

  DifferentialRotationData Load(const std::string& path)
  {
    LOG_FUNCTION("DifferentialRotation::Load");
    LOG_DEBUG("Loading differential rotation data {}", path);

    cv::FileStorage file(path, cv::FileStorage::READ);
    // diffrot params
    file["xsize"] >> xsize;
    file["ysize"] >> ysize;
    file["idstep"] >> idstep;
    file["idstride"] >> idstride;
    file["yfov"] >> yfov;
    file["cadence"] >> cadence;
    // diffrot data
    DifferentialRotationData data(xsize, ysize);
    file["thetas"] >> data.thetas;
    file["shiftsx"] >> data.shiftsx;
    file["shiftsy"] >> data.shiftsy;
    file["omegasx"] >> data.omegasx;
    file["omegasy"] >> data.omegasy;
    file["fshiftx"] >> data.fshiftx;
    file["fshifty"] >> data.fshifty;
    file["theta0s"] >> data.theta0s;
    file["Rs"] >> data.Rs;

    return data;
  }

  i32 xsize = 2500;
  i32 ysize = 851;
  i32 idstep = 1;
  i32 idstride = 25;
  i32 yfov = 3400;
  i32 cadence = 45;
};