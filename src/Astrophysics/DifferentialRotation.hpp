#pragma once
#include "IPC/IterativePhaseCorrelation.hpp"
#include "Fit/Polyfit.hpp"
#include "Fit/Trigfit.hpp"
#include "Utils/DataCache.hpp"
#include "Filtering/Median.hpp"

class DifferentialRotation
{
public:
  DifferentialRotation(i32 xsize_, i32 ysize_, i32 idstep_, i32 idstride_, f64 thetamax_, i32 cadence_) :
    xsize(xsize_), ysize(ysize_), idstep(idstep_), idstride(idstride_), thetamax(thetamax_), cadence(cadence_)
  {
    PROFILE_FUNCTION;
    if (idstride > 0) // images are not reused with non-zero stride
    {
      imageCache.SetCapacity(0);
      headerCache.SetCapacity(0);
    }
  }

  struct DifferentialRotationData
  {
    DifferentialRotationData(i32 xsize, i32 ysize, f64 thetamax) :
      shiftx(cv::Mat::zeros(ysize, xsize, CV_32F)),
      shifty(cv::Mat::zeros(ysize, xsize, CV_32F)),
      omegax(cv::Mat::zeros(ysize, xsize, CV_32F)),
      omegay(cv::Mat::zeros(ysize, xsize, CV_32F)),
      theta(GenerateTheta(ysize, thetamax)),
      fshiftx(std::vector<f64>(xsize, 0.)),
      fshifty(std::vector<f64>(xsize, 0.)),
      theta0(std::vector<f64>(xsize, 0.)),
      R(std::vector<f64>(xsize, 0.))
    {
    }

    static std::vector<f64> GenerateTheta(i32 ysize, f64 thetamax)
    {
      std::vector<f64> theta(ysize);
      const auto thetastep = thetamax * 2 / (ysize - 1);
      for (usize i = 0; i < theta.size(); ++i)
        theta[i] = thetamax - i * thetastep;
      return theta;
    }

    void PostProcess()
    {
      PROFILE_FUNCTION;
      const i32 medsizeX = std::min(3, shiftx.cols); // time
      const i32 medsizeY = std::min(3, shiftx.rows); // meridian
      shiftx = MedianBlur<f32>(shiftx, medsizeX, medsizeY);
      shifty = MedianBlur<f32>(shifty, medsizeX, medsizeY);
      omegax = MedianBlur<f32>(omegax, medsizeX, medsizeY);
      omegay = MedianBlur<f32>(omegay, medsizeX, medsizeY);
    }

    cv::Mat shiftx, shifty, omegax, omegay;
    std::vector<f64> theta, fshiftx, fshifty, theta0, R;
  };

  template <bool Managed = false> // executed automatically by some logic (e.g. optimization algorithm) instead of manually
  DifferentialRotationData Calculate(const IterativePhaseCorrelation& ipc, const std::string& dataPath, i32 idstart) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(not Managed, "DifferentialRotation::Calculate");
    DifferentialRotationData data(xsize, ysize, thetamax);
    std::atomic<i32> progress = 0;
    const auto tstep = idstep * cadence;
    const auto wxsize = ipc.GetCols();
    const auto wysize = ipc.GetRows();
    const auto shiftxmin = 0.01 * idstep;
    const auto shiftxmax = 0.4 * idstep;
    const auto shiftymax = 0.08;
    const auto fshiftmax = 0.1;
    const auto ids = GenerateIds(idstart);
    const auto omegaxpred = GetPredictedOmegas(data.theta, 14.296, -1.847, -2.615);

#pragma omp parallel for if (not Managed)
    for (i32 x = 0; x < xsize; ++x)
      try
      {
        PROFILE_SCOPE(CalculateMeridianShifts);
        const auto logprogress = static_cast<f64>(progress.fetch_add(1, std::memory_order_relaxed)) + 1.;
        const auto [id1, id2] = ids[x];
        const auto path1 = fmt::format("{}/{}.png", dataPath, id1);
        const auto path2 = fmt::format("{}/{}.png", dataPath, id2);
        if (std::filesystem::exists(path1) and std::filesystem::exists(path2))
          [[likely]]
          {
            if constexpr (not Managed)
              LOG_DEBUG("[{:>3.0f}% :: {} / {}] Calculating diffrot profile {} - {} ...", logprogress / xsize * 100, logprogress, xsize, id1, id2);
          }
        else
          [[unlikely]]
          {
            if constexpr (not Managed)
              LOG_WARNING("[{:>3.0f}% :: {} / {}] Could not load images {} - {}, skipping ...", logprogress / xsize * 100, logprogress, xsize, id1, id2);
            continue;
          }

        const auto image1 = imageCache.Get(path1);
        const auto image2 = imageCache.Get(path2);
        const auto header1 = headerCache.Get(fmt::format("{}/{}.json", dataPath, id1));
        const auto header2 = headerCache.Get(fmt::format("{}/{}.json", dataPath, id2));
        const auto theta0 = (header1.theta0 + header2.theta0) / 2;
        const auto R = (header1.R + header2.R) / 2;
        const auto xindex = xsize - 1 - x;
        data.fshiftx[xindex] = header2.xcenter - header1.xcenter;
        data.fshifty[xindex] = header2.ycenter - header1.ycenter;
        data.theta0[xindex] = theta0;
        data.R[xindex] = R;

        if (std::abs(data.fshiftx[xindex]) > fshiftmax or std::abs(data.fshifty[xindex]) > fshiftmax)
          [[unlikely]] continue;

        for (i32 y = 0; y < ysize; ++y)
        {
          PROFILE_SCOPE(CalculateMeridianShift);
          const auto theta = data.theta[y];
          const auto yshift = -R * std::sin(theta - theta0);
          auto crop1 = RoiCrop(image1, std::round(header1.xcenter), std::round(header1.ycenter + yshift), wxsize, wysize);
          auto crop2 = RoiCrop(image2, std::round(header2.xcenter), std::round(header2.ycenter + yshift), wxsize, wysize);
          const auto shift = ipc.Calculate<{.AccuracyT = IterativePhaseCorrelation::AccuracyType::SubpixelIterative}>(std::move(crop1), std::move(crop2));
          const auto shiftx = std::clamp(shift.x, shiftxmin, shiftxmax);
          const auto shifty = std::clamp(shift.y, -shiftymax, shiftymax);
          const auto omegax = std::clamp(std::asin(shiftx / (R * std::cos(theta))) / tstep * Constants::RadPerSecToDegPerDay, 0.7 * omegaxpred[y], 1.3 * omegaxpred[y]);
          const auto omegay = (std::asin((R * std::sin(theta) + shifty) / R) - theta) / tstep * Constants::RadPerSecToDegPerDay;

          data.shiftx.at<f32>(y, xindex) = shiftx;
          data.shifty.at<f32>(y, xindex) = shifty;
          data.omegax.at<f32>(y, xindex) = omegax;
          data.omegay.at<f32>(y, xindex) = omegay;
        }
      }
      catch (const std::exception& e)
      {
        if constexpr (not Managed)
          LOG_WARNING("DifferentialRotation::Calculate error: {} - skipping ...", e.what());
        continue;
      }

    FixMissingData<Managed>(data);
    data.PostProcess();

    if constexpr (not Managed)
      if (xsize > 100)
        Save(data, ipc, fmt::format("{}/proc", dataPath));

    if constexpr (not Managed)
      Plot(data, dataPath, idstart);

    return data;
  }

  void LoadAndShow(const std::string& path, const std::string& dataPath, i32 idstart)
  try
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("DifferentialRotation::LoadAndShow");
    auto data = Load(path);
    Plot(data, dataPath, idstart);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("DifferentialRotation::LoadAndShow error: {}", e.what());
  }

  void Optimize(IterativePhaseCorrelation& ipc, const std::string& dataPath, i32 idstart, i32 xsizeopt, i32 ysizeopt, i32 popsize) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("DifferentialRotation::Optimize");
    i32 idstrideopt = idstride * std::floor(static_cast<f64>(xsize) / xsizeopt); // automatically stretch opt samples over hte entire time span
    LOG_INFO("Optimization xsize: {}", xsizeopt);
    LOG_INFO("Optimization ysize: {}", ysizeopt);
    LOG_INFO("Optimization popsize: {}", popsize);
    LOG_INFO("Optimization idstride: {}", idstrideopt);
    DifferentialRotation diffrot(xsizeopt, ysizeopt, idstep, idstrideopt, thetamax, cadence);
    const usize ids = idstride > 0 ? xsizeopt * 2 : xsizeopt + 1;
    diffrot.imageCache.SetGetDataFunction([](const std::string& path) {
      PROFILE_SCOPE(Imread);
      return LoadUnitFloatImage<IterativePhaseCorrelation::Float>(path); // cache images already converted to desired format for IPC
    });
    diffrot.imageCache.Reserve(ids);
    diffrot.headerCache.Reserve(ids);

    const auto dataBefore = diffrot.Calculate<true>(ipc, dataPath, idstart);
    const auto predfit = GetVectorAverage({GetPredictedOmegas(dataBefore.theta, 14.296, -1.847, -2.615), GetPredictedOmegas(dataBefore.theta, 14.192, -1.70, -2.36)});
    const auto obj = [&](const IterativePhaseCorrelation& ipcopt) {
      const auto dataopt = diffrot.Calculate<true>(ipcopt, dataPath, idstart);
      const auto omegax = GetRowAverage(dataopt.omegax);
      const auto omegaxfit = polyfit(dataopt.theta, omegax, 2);

      f64 ret = 0;
      for (usize i = 0; i < omegaxfit.size(); ++i)
      {
        ret += 0.7 * std::pow(omegaxfit[i] - predfit[i], 2); // minimize pred fit diff
        ret += 0.3 * std::pow(omegax[i] - omegaxfit[i], 2);  // minimize variance
      }
      return ret / omegaxfit.size();
    };

    ipc.Optimize(obj, popsize);
    if (xsizeopt >= 100)
      SaveOptimizedParameters(ipc, fmt::format("{}/proc", dataPath), xsizeopt, ysizeopt, popsize);
    const auto dataAfter = diffrot.Calculate<true>(ipc, dataPath, idstart);

    PyPlot::Plot("Diffrot opt",
        {.x = Constants::Rad * dataAfter.theta,
            .ys = {GetRowAverage(dataBefore.omegax), GetRowAverage(dataAfter.omegax), polyfit(dataAfter.theta, GetRowAverage(dataAfter.omegax), 2),
                sin2sin4fit(dataAfter.theta, GetRowAverage(dataAfter.omegax)), GetPredictedOmegas(dataAfter.theta, 14.296, -1.847, -2.615), GetPredictedOmegas(dataAfter.theta, 14.192, -1.70, -2.36)},
            .xlabel = "latitude [deg]",
            .ylabel = "average omega x [deg/day]",
            .label_ys = {"ipc", "ipc opt", "ipc opt polyfit", "ipc opt trigfit", "Derek A. Lamb (2017)", "Howard et al. (1983)"}});
  }

  static void PlotMeridianCurve(const DifferentialRotationData& data, const std::string& dataPath, i32 idstart, f64 timestep)
  {
    PROFILE_FUNCTION;
    const auto image = cv::imread(fmt::format("{}/{}.png", dataPath, idstart), cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
    const auto header = GetHeader(fmt::format("{}/{}.json", dataPath, idstart));
    const auto omegax = GetRowAverage(data.omegax);                            // [deg/day]
    const auto predx = GetPredictedOmegas(data.theta, 14.296, -1.847, -2.615); // [deg/day]
    std::vector<cv::Point2d> mcpts(data.theta.size());                         // [px,px]
    std::vector<cv::Point2d> mcptspred(data.theta.size());                     // [px,px]
    std::vector<cv::Point2d> mcptsz(data.theta.size());                        // [px,px]

    for (usize y = 0; y < data.theta.size(); ++y)
    {
      const f64 mcx = header.xcenter + header.R * std::cos(data.theta[y]) * std::sin(omegax[y] * timestep / Constants::Rad);
      const f64 mcy = header.ycenter - header.R * std::sin(data.theta[y] - header.theta0);
      mcpts[y] = cv::Point2d(mcx, mcy);

      const f64 mcxpred = header.xcenter + header.R * std::cos(data.theta[y]) * std::sin(predx[y] * timestep / Constants::Rad);
      const f64 mcypred = header.ycenter - header.R * std::sin(data.theta[y] - header.theta0);
      mcptspred[y] = cv::Point2d(mcxpred, mcypred);

      const f64 mcxz = header.xcenter + header.R * std::cos(data.theta[y]) * std::sin(predx[y] * 0 / Constants::Rad);
      const f64 mcyz = header.ycenter - header.R * std::sin(data.theta[y] - header.theta0);
      mcptsz[y] = cv::Point2d(mcxz, mcyz);
    }

    cv::Mat imageclr, imageclrz;
    cv::cvtColor(image, imageclr, cv::COLOR_GRAY2RGB);
    cv::cvtColor(image, imageclrz, cv::COLOR_GRAY2RGB);
    const auto thickness = 13;
    const auto color = 65535. / 255 * cv::Scalar(50., 205., 50.);
    const auto colorpred = 65535. / 255 * cv::Scalar(255., 0, 255.);
    for (usize y = 0; y < mcpts.size() - 1; ++y)
    {
      if (data.omegax.cols > 10)
      {
        const auto pt1 = mcpts[y];
        const auto pt2 = mcpts[y + 1];
        cv::line(imageclr, pt1, pt2, color, thickness, cv::LINE_AA);
      }

      const auto pt1pred = mcptspred[y];
      const auto pt2pred = mcptspred[y + 1];
      cv::line(imageclr, pt1pred, pt2pred, colorpred, thickness, cv::LINE_AA);

      const auto pt1z = mcptsz[y];
      const auto pt2z = mcptsz[y + 1];
      cv::line(imageclrz, pt1z, pt2z, colorpred, thickness, cv::LINE_AA);
    }

    Showimg(imageclr, "meridian curve", false, 0, 1, 1200);
    // Saveimg(fmt::format("{}/meridian_curve.png", "../debug/debug"), imageclr, false, imageclr.size() / 6);
  }

private:
  struct ImageHeader
  {
    f64 xcenter; // [px]
    f64 ycenter; // [px]
    f64 theta0;  // [rad]
    f64 R;       // [px]
  };

  i32 xsize = 2500;
  i32 ysize = 101;
  i32 idstep = 1;
  i32 idstride = 25;
  f64 thetamax = 50. / Constants::Rad;
  i32 cadence = 45;
  mutable DataCache<std::string, cv::Mat> imageCache{[](const std::string& path) {
    PROFILE_SCOPE(Imread);
    return cv::imread(path, cv::IMREAD_UNCHANGED);
  }};
  mutable DataCache<std::string, ImageHeader> headerCache{[](const std::string& path) { return GetHeader(path); }};

  static ImageHeader GetHeader(const std::string& path)
  {
    PROFILE_FUNCTION;
    std::ifstream file(path);
    json::json j;
    file >> j;

    ImageHeader header;
    header.xcenter = (j["NAXIS1"].get<f64>()) - (j["CRPIX1"].get<f64>()); // [py] (x is flipped, 4095 - fits index from 1)
    header.ycenter = j["CRPIX2"].get<f64>() - 1;                          // [px] (fits index from 1)
    header.theta0 = j["CRLT_OBS"].get<f64>() / Constants::Rad;            // [rad] (convert from deg to rad)
    header.R = j["RSUN_OBS"].get<f64>() / j["CDELT1"].get<f64>();         // [px] (arcsec / arcsec per pixel)
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

  template <bool Managed>
  static void FixMissingData(DifferentialRotationData& data)
  {
    PROFILE_FUNCTION;
    // fix missing data by interpolation
    for (i32 x = 0; x < data.omegax.cols; ++x)
    {
      if (data.omegax.at<f32>(0, x) != 0.0f) // no need to fix, data not missing
        continue;

      // find first non-missing previous data
      auto xindex1 = std::max(x - 1, 0);

      // find first non-missing next data
      auto xindex2 = std::min(x + 1, data.omegax.cols - 1);
      while (data.omegax.at<f32>(0, xindex2) == 0.0f and xindex2 < data.omegax.cols - 1)
        ++xindex2;

      const f64 t = (static_cast<f64>(x) - xindex1) / (xindex2 - xindex1);

      if constexpr (not Managed)
        LOG_DEBUG("Fixing missing data: {} < x({}) < {}, t: {:.2f} ...", xindex1, x, xindex2, t);

      for (i32 y = 0; y < data.omegax.rows; ++y)
      {
        data.fshiftx[x] = std::lerp(data.fshiftx[xindex1], data.fshiftx[xindex2], t);
        data.fshifty[x] = std::lerp(data.fshifty[xindex1], data.fshifty[xindex2], t);
        data.theta0[x] = std::lerp(data.theta0[xindex1], data.theta0[xindex2], t);
        data.R[x] = std::lerp(data.R[xindex1], data.R[xindex2], t);
        data.shiftx.at<f32>(y, x) = std::lerp(data.shiftx.at<f32>(y, xindex1), data.shiftx.at<f32>(y, xindex2), static_cast<f32>(t));
        data.shifty.at<f32>(y, x) = std::lerp(data.shifty.at<f32>(y, xindex1), data.shifty.at<f32>(y, xindex2), static_cast<f32>(t));
        data.omegax.at<f32>(y, x) = std::lerp(data.omegax.at<f32>(y, xindex1), data.omegax.at<f32>(y, xindex2), static_cast<f32>(t));
        data.omegay.at<f32>(y, x) = std::lerp(data.omegay.at<f32>(y, xindex1), data.omegay.at<f32>(y, xindex2), static_cast<f32>(t));
      }
    }
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

  static std::vector<f64> GetRowAverage(const cv::Mat& vals)
  {
    PROFILE_FUNCTION;
    std::vector<f64> avgs(vals.rows, 0.);

    for (i32 y = 0; y < vals.rows; ++y)
      for (i32 x = 0; x < vals.cols; ++x)
        avgs[y] += vals.at<f32>(y, x);

    for (i32 y = 0; y < vals.rows; ++y)
      avgs[y] /= vals.cols;

    return avgs;
  }

  static std::vector<f64> GetVectorAverage(const std::vector<std::vector<f64>>& vecs)
  {
    PROFILE_FUNCTION;
    std::vector<f64> avg(vecs[0].size());

    for (usize v = 0; v < vecs.size(); ++v)
      for (usize i = 0; i < vecs[v].size(); ++i)
        avg[i] += vecs[v][i];

    for (usize i = 0; i < avg.size(); ++i)
      avg[i] /= vecs.size();

    return avg;
  }

  static std::vector<f64> GetPredictedOmegas(const std::vector<f64>& theta, f64 A, f64 B, f64 C)
  {
    PROFILE_FUNCTION;
    std::vector<f64> omegas(theta.size());
    for (usize i = 0; i < theta.size(); ++i)
      omegas[i] = A + B * std::pow(std::sin(theta[i]), 2) + C * std::pow(std::sin(theta[i]), 4);
    return omegas;
  }

  void Plot(DifferentialRotationData& data, const std::string& dataPath, i32 idstart) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("DifferentialRotation::Plot");
    const auto times = GetTimesInDays(idstep * cadence, idstride * cadence, xsize);
    PlotMeridianCurve(data, dataPath, idstart, 27);

    PyPlot::Plot("fits params", {.x = times,
                                    .ys = {data.fshiftx, data.fshifty},
                                    .y2s = {Constants::Rad * data.theta0},
                                    .xlabel = "time [days]",
                                    .ylabel = "fits shift [px]",
                                    .y2label = "theta0 [deg]",
                                    .label_ys = {"center shift x", "center shift y"},
                                    .label_y2s = {"theta0"}});
    PyPlot::Plot("avgshift x", {.x = Constants::Rad * data.theta,
                                   .y = GetRowAverage(data.shiftx),
                                   .y2 = GetRowAverage(data.shifty),
                                   .xlabel = "latitude [deg]",
                                   .ylabel = "average shift x [px]",
                                   .y2label = "average shift y [px]",
                                   .label_y = "ipc x",
                                   .label_y2 = "ipc y"});
    PyPlot::Plot("avgomega x", {
                                   .x = Constants::Rad * data.theta,
                                   .ys = {GetRowAverage(data.omegax), sin2sin4fit(data.theta, GetRowAverage(data.omegax)), polyfit(data.theta, GetRowAverage(data.omegax), 2),
                                       GetPredictedOmegas(data.theta, 14.296, -1.847, -2.615), GetPredictedOmegas(data.theta, 14.192, -1.70, -2.36)},
                                   .xlabel = "latitude [deg]",
                                   .ylabel = "average omega x [deg/day]",
                                   .label_ys = {"ipc", "ipc trigfit", "ipc polyfit", "Derek A. Lamb (2017)", "Howard et al. (1983)"},
                               });
    PyPlot::Plot("avgomega y", {
                                   .x = Constants::Rad * data.theta,
                                   .ys = {GetRowAverage(data.omegay), polyfit(data.theta, GetRowAverage(data.omegay), 3)},
                                   .xlabel = "latitude [deg]",
                                   .ylabel = "average omega x [deg/day]",
                                   .label_ys = {"ipc", "ipc polyfit"},
                               });

    const f64 xmin = times.front(), xmax = times.back();
    const f64 ymin = data.theta.back() * Constants::Rad, ymax = data.theta.front() * Constants::Rad;
    const std::string xlabel = "time [days]";
    const std::string ylabel = "latitude [deg]";
    const f64 aspectratio = 2;
    PyPlot::Plot("shift x", {.z = data.shiftx, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = "shift x [px]", .aspectratio = aspectratio});
    PyPlot::Plot("omega x", {.z = data.omegax, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = "omega x [px]", .aspectratio = aspectratio});
    PyPlot::Plot("shift y", {.z = data.shifty, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = "shift y [px]", .aspectratio = aspectratio});
    PyPlot::Plot("omega y", {.z = data.omegay, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = "omega y [px]", .aspectratio = aspectratio});
  }

  void Save(const DifferentialRotationData& data, const IterativePhaseCorrelation& ipc, const std::string& dataPath) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("DifferentialRotation::Save");

    std::string path = fmt::format("{}/diffrot.json", dataPath);
    LOG_DEBUG("Saving differential rotation results to {} ...", std::filesystem::weakly_canonical(path).string());
    cv::FileStorage file(path, cv::FileStorage::WRITE);

    // diffrot params
    file << "xsize" << xsize;
    file << "ysize" << ysize;
    file << "idstep" << idstep;
    file << "idstride" << idstride;
    file << "thetamax" << thetamax;
    file << "cadence" << cadence;
    // ipc params
    file << "wxsize" << ipc.GetCols();
    file << "wysize" << ipc.GetRows();
    file << "bandpassL" << ipc.GetBandpassL();
    file << "bandpassH" << ipc.GetBandpassH();
    file << "L2size" << ipc.GetL2size();
    file << "L1ratio" << ipc.GetL1ratio();
    file << "L2Usize" << ipc.GetL2Usize();
    file << "BandpassType" << ipc.BandpassType2String(ipc.GetBandpassType());
    file << "WindowType" << ipc.WindowType2String(ipc.GetWindowType());
    file << "InterpolationType" << ipc.InterpolationType2String(ipc.GetInterpolationType());
    // diffrot data
    file << "theta" << data.theta;
    file << "shiftx" << data.shiftx;
    file << "shifty" << data.shifty;
    file << "omegax" << data.omegax;
    file << "omegay" << data.omegay;
    file << "fshiftx" << data.fshiftx;
    file << "fshifty" << data.fshifty;
    file << "theta0" << data.theta0;
    file << "R" << data.R;
  }

  void SaveOptimizedParameters(const IterativePhaseCorrelation& ipc, const std::string& dataPath, i32 xsizeopt, i32 ysizeopt, i32 popsize) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("DifferentialRotation::SaveOptimizedParameters");
    std::string path = fmt::format("{}/diffrot_ipcopt.json", dataPath);
    LOG_DEBUG("Saving differential rotation IPC optimization results to {} ...", std::filesystem::weakly_canonical(path).string());

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
    file << "L2Usize" << ipc.GetL2Usize();
    file << "BandpassType" << ipc.BandpassType2String(ipc.GetBandpassType());
    file << "WindowType" << ipc.WindowType2String(ipc.GetWindowType());
    file << "InterpolationType" << ipc.InterpolationType2String(ipc.GetInterpolationType());
  }

  DifferentialRotationData Load(const std::string& path)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("DifferentialRotation::Load");
    LOG_DEBUG("Loading differential rotation data {}", path);

    cv::FileStorage file(path, cv::FileStorage::READ);
    // diffrot params
    file["xsize"] >> xsize;
    file["ysize"] >> ysize;
    file["idstep"] >> idstep;
    file["idstride"] >> idstride;
    file["thetamax"] >> thetamax;
    file["cadence"] >> cadence;
    // diffrot data
    DifferentialRotationData data(xsize, ysize, thetamax);
    file["theta"] >> data.theta;
    file["shiftx"] >> data.shiftx;
    file["shifty"] >> data.shifty;
    file["omegax"] >> data.omegax;
    file["omegay"] >> data.omegay;
    file["fshiftx"] >> data.fshiftx;
    file["fshifty"] >> data.fshifty;
    file["theta0"] >> data.theta0;
    file["R"] >> data.R;

    return data;
  }
};
