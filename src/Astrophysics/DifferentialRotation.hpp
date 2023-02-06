#pragma once
#include "ImageRegistration/IPC.hpp"
#include "Math/PolynomialFit.hpp"
#include "Math/TrigonometricFit.hpp"
#include "Utils/DataCache.hpp"
#include "ImageProcessing/MedianBlur.hpp"

class DifferentialRotation
{
  static constexpr f64 SecondsInDay = 24. * 60. * 60.;
  static constexpr f64 RadPerSecToDegPerDay = ToDegrees(1) * SecondsInDay;

public:
  struct ImageHeader
  {
    f64 xcenter; // [px]
    f64 ycenter; // [px]
    f64 theta0;  // [rad]
    f64 R;       // [px]
  };

  struct DifferentialRotationData
  {
    DifferentialRotationData() {}

    DifferentialRotationData(i32 xsize_, i32 ysize_, i32 idstep_, i32 idstride_, f64 thetamax_, i32 cadence_, i32 idstart_) :
      xsize(xsize_),
      ysize(ysize_),
      idstep(idstep_),
      idstride(idstride_),
      thetamax(thetamax_),
      cadence(cadence_),
      idstart(idstart_),
      shiftx(cv::Mat::zeros(ysize_, xsize_, CV_32F)),
      shifty(cv::Mat::zeros(ysize_, xsize_, CV_32F)),
      omegax(cv::Mat::zeros(ysize_, xsize_, CV_32F)),
      omegay(cv::Mat::zeros(ysize_, xsize_, CV_32F)),
      theta(GenerateTheta(ysize_, thetamax_)),
      fshiftx(std::vector<f64>(xsize_, 0.)),
      fshifty(std::vector<f64>(xsize_, 0.)),
      theta0(std::vector<f64>(xsize_, 0.)),
      R(std::vector<f64>(xsize_, 0.))
    {
    }

    void Load(const std::string& path)
    {
      PROFILE_FUNCTION;
      LOG_FUNCTION;
      cv::FileStorage file(path, cv::FileStorage::READ);
      file["xsize"] >> xsize;
      file["ysize"] >> ysize;
      file["idstep"] >> idstep;
      file["idstride"] >> idstride;
      file["thetamax"] >> thetamax;
      file["cadence"] >> cadence;
      file["theta"] >> theta;
      file["shiftx"] >> shiftx;
      file["shifty"] >> shifty;
      file["omegax"] >> omegax;
      file["omegay"] >> omegay;
      file["fshiftx"] >> fshiftx;
      file["fshifty"] >> fshifty;
      file["theta0"] >> theta0;
      file["R"] >> R;
    }

    void Save(const std::string& dataPath, const IPC& ipc) const
    {
      PROFILE_FUNCTION;
      LOG_FUNCTION;
      std::string path = fmt::format("{}/diffrot.json", dataPath);
      LOG_DEBUG("Saving differential rotation results to {}", std::filesystem::weakly_canonical(path).string());
      cv::FileStorage file(path, cv::FileStorage::WRITE);
      file << "xsize" << xsize;
      file << "ysize" << ysize;
      file << "idstep" << idstep;
      file << "idstride" << idstride;
      file << "thetamax" << thetamax;
      file << "cadence" << cadence;
      file << "IPC" << ipc.Serialize();
      file << "theta" << theta;
      file << "shiftx" << shiftx;
      file << "shifty" << shifty;
      file << "omegax" << omegax;
      file << "omegay" << omegay;
      file << "fshiftx" << fshiftx;
      file << "fshifty" << fshifty;
      file << "theta0" << theta0;
      file << "R" << R;
    }

    static std::vector<f64> GenerateTheta(i32 ysize, f64 thetamax)
    {
      std::vector<f64> theta(ysize);
      const auto thetastep = thetamax * 2 / (ysize - 1);
      for (usize i = 0; i < theta.size(); ++i)
        theta[i] = thetamax - i * thetastep;
      return theta;
    }

    std::vector<std::pair<i32, i32>> GenerateIds() const
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

    template <bool Managed>
    void FixMissingData()
    {
      PROFILE_FUNCTION;
      // fix missing data by interpolation
      for (i32 x = 0; x < omegax.cols; ++x)
      {
        if (omegax.at<f32>(0, x) != 0.0f) // no need to fix, data not missing
          continue;

        // find first non-missing previous data
        auto xindex1 = std::max(x - 1, 0);

        // find first non-missing next data
        auto xindex2 = std::min(x + 1, omegax.cols - 1);
        while (omegax.at<f32>(0, xindex2) == 0.0f and xindex2 < omegax.cols - 1)
          ++xindex2;

        const f64 t = (static_cast<f64>(x) - xindex1) / (xindex2 - xindex1);

        if constexpr (not Managed)
          LOG_DEBUG("Fixing missing data: {} < x({}) < {}, t: {:.2f}", xindex1, x, xindex2, t);

        for (i32 y = 0; y < omegax.rows; ++y)
        {
          fshiftx[x] = std::lerp(fshiftx[xindex1], fshiftx[xindex2], t);
          fshifty[x] = std::lerp(fshifty[xindex1], fshifty[xindex2], t);
          theta0[x] = std::lerp(theta0[xindex1], theta0[xindex2], t);
          R[x] = std::lerp(R[xindex1], R[xindex2], t);
          shiftx.at<f32>(y, x) = std::lerp(shiftx.at<f32>(y, xindex1), shiftx.at<f32>(y, xindex2), static_cast<f32>(t));
          shifty.at<f32>(y, x) = std::lerp(shifty.at<f32>(y, xindex1), shifty.at<f32>(y, xindex2), static_cast<f32>(t));
          omegax.at<f32>(y, x) = std::lerp(omegax.at<f32>(y, xindex1), omegax.at<f32>(y, xindex2), static_cast<f32>(t));
          omegay.at<f32>(y, x) = std::lerp(omegay.at<f32>(y, xindex1), omegay.at<f32>(y, xindex2), static_cast<f32>(t));
        }
      }
    }

    i32 xsize = 2500;
    i32 ysize = 101;
    i32 idstep = 1;
    i32 idstride = 25;
    f64 thetamax = ToRadians(50);
    i32 cadence = 45;
    i32 idstart = 123456;

    cv::Mat shiftx, shifty, omegax, omegay;
    std::vector<f64> theta, fshiftx, fshifty, theta0, R;
  };

  template <bool Managed = false> // executed automatically by some logic (e.g.
                                  // optimization algorithm) instead of manually
  static DifferentialRotationData Calculate(
      const IPC& ipc, const std::string& dataPath, i32 xsize, i32 ysize, i32 idstep, i32 idstride, f64 thetamax, i32 cadence, i32 idstart, f32* progress = nullptr)
  {
    PROFILE_FUNCTION;
    if constexpr (not Managed)
      LOG_FUNCTION;

    DataCache<std::string, cv::Mat> imageCache{[](const std::string& path)
        {
          PROFILE_SCOPE(Imread);
          return cv::imread(path, cv::IMREAD_UNCHANGED);
        }};
    DataCache<std::string, ImageHeader> headerCache{[](const std::string& path) { return GetHeader(path); }};

    return Calculate<Managed>(ipc, dataPath, xsize, ysize, idstep, idstride, thetamax, cadence, idstart, progress, imageCache, headerCache);
  }

  template <bool Managed = false> // executed automatically by some logic (e.g.
                                  // optimization algorithm) instead of manually
  static DifferentialRotationData Calculate(const IPC& ipc, const std::string& dataPath, i32 xsize, i32 ysize, i32 idstep, i32 idstride, f64 thetamax, i32 cadence, i32 idstart,
      f32* progress, DataCache<std::string, cv::Mat>& imageCache, DataCache<std::string, ImageHeader>& headerCache)
  {
    PROFILE_FUNCTION;
    if constexpr (not Managed)
      LOG_FUNCTION;

    DifferentialRotationData data(xsize, ysize, idstep, idstride, thetamax, cadence, idstart);
    std::atomic<i32> progressi = 0;
    const auto tstep = idstep * cadence;
    const auto wxsize = ipc.GetCols();
    const auto wysize = ipc.GetRows();
    const auto shiftxmin = 0.01 * idstep;
    const auto shiftxmax = 0.4 * idstep;
    const auto shiftymax = 0.08;
    const auto fshiftmax = 0.1;
    const auto ids = data.GenerateIds();
    const auto omegaxpred = GetPredictedOmegas(data.theta, 14.296, -1.847, -2.615);

#pragma omp parallel for if (not Managed)
    for (i32 x = 0; x < xsize; ++x)
      try
      {
        PROFILE_SCOPE(CalculateMeridianShifts);
        const f64 logprogress = ++progressi;
        if (progress)
          *progress = logprogress / xsize;
        const auto [id1, id2] = ids[x];
        const auto path1 = fmt::format("{}/{}.png", dataPath, id1);
        const auto path2 = fmt::format("{}/{}.png", dataPath, id2);
        if (std::filesystem::exists(path1) and std::filesystem::exists(path2)) [[likely]]
        {
          if constexpr (not Managed)
            LOG_DEBUG("[{:>3.0f}% :: {} / {}] Calculating diffrot profile {} - "
                      "{}",
                logprogress / xsize * 100, logprogress, xsize, id1, id2);
        }
        else [[unlikely]]
        {
          if constexpr (not Managed)
            LOG_WARNING("[{:>3.0f}% :: {} / {}] Could not load images {} - {}, "
                        "skipping",
                logprogress / xsize * 100, logprogress, xsize, id1, id2);
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

        if (std::abs(data.fshiftx[xindex]) > fshiftmax or std::abs(data.fshifty[xindex]) > fshiftmax) [[unlikely]]
          continue;

        for (i32 y = 0; y < ysize; ++y)
        {
          PROFILE_SCOPE(CalculateMeridianShift);
          const auto theta = data.theta[y];
          const auto yshift = -R * std::sin(theta - theta0);
          auto crop1 = RoiCrop(image1, std::round(header1.xcenter), std::round(header1.ycenter + yshift), wxsize, wysize);
          auto crop2 = RoiCrop(image2, std::round(header2.xcenter), std::round(header2.ycenter + yshift), wxsize, wysize);
          const auto shift = ipc.Calculate(std::move(crop1), std::move(crop2));
          const auto shiftx = std::clamp(shift.x, shiftxmin, shiftxmax);
          const auto shifty = std::clamp(shift.y, -shiftymax, shiftymax);
          const auto omegax = std::clamp(std::asin(shiftx / (R * std::cos(theta))) / tstep * RadPerSecToDegPerDay, 0.7 * omegaxpred[y], 1.3 * omegaxpred[y]);
          const auto omegay = (std::asin((R * std::sin(theta) + shifty) / R) - theta) / tstep * RadPerSecToDegPerDay;

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

    data.FixMissingData<Managed>();
    data.PostProcess();

    if constexpr (not Managed)
    {
      if (xsize > 100)
        data.Save(fmt::format("{}/proc", dataPath), ipc);
      Plot(data, dataPath);
    }

    return data;
  }

  static void Optimize(
      IPC& ipc, const std::string& dataPath, i32 xsize, i32 ysize, i32 idstep, i32 idstride, f64 thetamax, i32 cadence, i32 idstart, i32 xsizeopt, i32 ysizeopt, i32 popsize)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION;
    i32 idstrideopt = idstride * std::floor(static_cast<f64>(xsize) / xsizeopt); // automatically stretch opt samples
                                                                                 // over the entire time span
    LOG_INFO("Optimization xsize: {}", xsizeopt);
    LOG_INFO("Optimization ysize: {}", ysizeopt);
    LOG_INFO("Optimization popsize: {}", popsize);
    LOG_INFO("Optimization idstride: {}", idstrideopt);
    const usize ids = idstride > 0 ? xsizeopt * 2 : xsizeopt + 1;
    DataCache<std::string, cv::Mat> imageCache{[](const std::string& path)
        {
          PROFILE_SCOPE(Imread);
          return LoadUnitFloatImage<IPC::Float>(path); // cache images already converted to desired format for IPC
        }};
    DataCache<std::string, ImageHeader> headerCache{[](const std::string& path) { return GetHeader(path); }};
    imageCache.Reserve(ids);
    headerCache.Reserve(ids);

    const auto dataBefore = Calculate<true>(ipc, dataPath, xsizeopt, ysizeopt, idstep, idstride, thetamax, cadence, idstart, nullptr, imageCache, headerCache);
    const auto predfit = GetVectorAverage({GetPredictedOmegas(dataBefore.theta, 14.296, -1.847, -2.615), GetPredictedOmegas(dataBefore.theta, 14.192, -1.70, -2.36)});

    const auto obj = [&](const IPC& ipcopt)
    {
      const auto dataopt = Calculate<true>(ipc, dataPath, xsizeopt, ysizeopt, idstep, idstride, thetamax, cadence, idstart, nullptr, imageCache, headerCache);
      if (dataopt.omegax.empty())
        return std::numeric_limits<f64>::infinity();
      const auto omegax = GetRowAverage(dataopt.omegax);
      const auto omegaxfit = PolynomialFit(dataopt.theta, omegax, 2);

      f64 ret = 0;
      for (usize i = 0; i < omegaxfit.size(); ++i)
      {
        ret += 0.7 * std::pow(omegaxfit[i] - predfit[i], 2); // minimize pred fit diff
        ret += 0.3 * std::pow(omegax[i] - omegaxfit[i], 2);  // minimize variance
      }
      return ret / omegaxfit.size();
    };

    IPCOptimization::Optimize(ipc, obj, popsize);
    if (xsizeopt >= 100)
      SaveOptimizedParameters(ipc, fmt::format("{}/proc", dataPath), xsizeopt, ysizeopt, popsize);
    const auto dataAfter = Calculate<true>(ipc, dataPath, xsizeopt, ysizeopt, idstep, idstride, thetamax, cadence, idstart, nullptr, imageCache, headerCache);

    PyPlot::Plot({
        .name = "Diffrot opt",
        .x = ToDegrees(1) * dataAfter.theta,
        .ys = {GetRowAverage(dataBefore.omegax), GetRowAverage(dataAfter.omegax), PolynomialFit(dataAfter.theta, GetRowAverage(dataAfter.omegax), 2),
            TrigonometricFit(dataAfter.theta, GetRowAverage(dataAfter.omegax)), GetPredictedOmegas(dataAfter.theta, 14.296, -1.847, -2.615),
            GetPredictedOmegas(dataAfter.theta, 14.192, -1.70, -2.36)},
        .ylabels = {"ipc", "ipc opt", "ipc opt PolynomialFit", "ipc opt trigfit", "Derek A. Lamb (2017)", "Howard et al. (1983)"},
        .xlabel = "latitude [deg]",
        .ylabel = "average omega x [deg/day]",
    });
  }

  static void PlotMeridianCurve(const DifferentialRotationData& data, const std::string& dataPath, f64 timestep)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION;
    const auto image = cv::imread(fmt::format("{}/{}.png", dataPath, data.idstart), cv::IMREAD_GRAYSCALE | cv::IMREAD_UNCHANGED);
    const auto header = GetHeader(fmt::format("{}/{}.json", dataPath, data.idstart));
    const auto omegax = GetRowAverage(data.omegax);                            // [deg/day]
    const auto predx = GetPredictedOmegas(data.theta, 14.296, -1.847, -2.615); // [deg/day]
    std::vector<cv::Point2d> mcpts(data.theta.size());                         // [px,px]
    std::vector<cv::Point2d> mcptspred(data.theta.size());                     // [px,px]
    std::vector<cv::Point2d> mcptsz(data.theta.size());                        // [px,px]

    for (usize y = 0; y < data.theta.size(); ++y)
    {
      const f64 mcx = header.xcenter + header.R * std::cos(data.theta[y]) * std::sin(omegax[y] * timestep / ToDegrees(1));
      const f64 mcy = header.ycenter - header.R * std::sin(data.theta[y] - header.theta0);
      mcpts[y] = cv::Point2d(mcx, mcy);

      const f64 mcxpred = header.xcenter + header.R * std::cos(data.theta[y]) * std::sin(predx[y] * timestep / ToDegrees(1));
      const f64 mcypred = header.ycenter - header.R * std::sin(data.theta[y] - header.theta0);
      mcptspred[y] = cv::Point2d(mcxpred, mcypred);

      const f64 mcxz = header.xcenter + header.R * std::cos(data.theta[y]) * std::sin(predx[y] * 0 / ToDegrees(1));
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

    Plot::Plot("meridian curve", imageclr);
  }

  static void PlotGradualIdStep(const IPC& ipc, i32 maxstep)
  {
    LOG_FUNCTION;
    const std::string dataPath = "/media/zdenyhraz/Zdeny_exSSD/diffrot_day_2500";
    const i32 idstart = 18933122;
    const auto image1 = RoiCrop(LoadUnitFloatImage<IPC::Float>(fmt::format("{}/{}.png", dataPath, idstart)), 4096 / 2, 4096 / 2, ipc.GetCols(), ipc.GetRows());

    for (i32 idstep = 1; idstep <= maxstep; ++idstep)
    {
      const auto image2 = RoiCrop(LoadUnitFloatImage<IPC::Float>(fmt::format("{}/{}.png", dataPath, idstart + idstep)), 4096 / 2, 4096 / 2, ipc.GetCols(), ipc.GetRows());
      ipc.SetDebugName(fmt::format("{}s", idstep * 45));
      ipc.Calculate<IPC::Mode::Debug>(image1, image2);
    }
  }

private:
  static ImageHeader GetHeader(const std::string& path)
  {
    PROFILE_FUNCTION;
    std::ifstream file(path);
    json::json j;
    file >> j;

    ImageHeader header;
    header.xcenter = (j["NAXIS1"].get<f64>()) - (j["CRPIX1"].get<f64>()); // [py] (x is flipped, 4095 - fits index from 1)
    header.ycenter = j["CRPIX2"].get<f64>() - 1;                          // [px] (fits index from 1)
    header.theta0 = ToRadians(j["CRLT_OBS"].get<f64>());                  // [rad] (convert from deg to rad)
    header.R = j["RSUN_OBS"].get<f64>() / j["CDELT1"].get<f64>();         // [px] (arcsec / arcsec per pixel)
    return header;
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

  static void Plot(const DifferentialRotationData& data, const std::string& dataPath)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION;
    const auto times = GetTimesInDays(data.idstep * data.cadence, data.idstride * data.cadence, data.xsize);
    PlotMeridianCurve(data, dataPath, 27);

    PyPlot::Plot({
        .name = "fits params",
        .x = times,
        .ys = {data.fshiftx, data.fshifty},
        .y2s = {ToDegrees(1) * data.theta0},
        .ylabels = {"center shift x", "center shift y"},
        .y2labels = {"theta0"},
        .xlabel = "time [days]",
        .ylabel = "fits shift [px]",
        .y2label = "theta0 [deg]",
    });
    PyPlot::Plot({
        .name = "avgshift x",
        .x = ToDegrees(1) * data.theta,
        .ys = {GetRowAverage(data.shiftx)},
        .y2s = {GetRowAverage(data.shifty)},
        .ylabels = {"ipc x"},
        .y2labels = {"ipc y"},
        .xlabel = "latitude [deg]",
        .ylabel = "average shift x [px]",
        .y2label = "average shift y [px]",
    });
    PyPlot::Plot({
        .name = "avgomega x",
        .x = ToDegrees(1) * data.theta,
        .ys = {GetRowAverage(data.omegax), TrigonometricFit(data.theta, GetRowAverage(data.omegax)), PolynomialFit(data.theta, GetRowAverage(data.omegax), 2),
            GetPredictedOmegas(data.theta, 14.296, -1.847, -2.615), GetPredictedOmegas(data.theta, 14.192, -1.70, -2.36)},
        .ylabels = {"ipc", "ipc trigfit", "ipc PolynomialFit", "Derek A. Lamb (2017)", "Howard et al. (1983)"},
        .xlabel = "latitude [deg]",
        .ylabel = "average omega x [deg/day]",
    });
    PyPlot::Plot({
        .name = "avgomega y",
        .x = ToDegrees(1) * data.theta,
        .ys = {GetRowAverage(data.omegay), PolynomialFit(data.theta, GetRowAverage(data.omegay), 3)},
        .ylabels = {"ipc", "ipc PolynomialFit"},
        .xlabel = "latitude [deg]",
        .ylabel = "average omega x [deg/day]",
    });

    const f64 xmin = times.front(), xmax = times.back();
    const f64 ymin = ToDegrees(data.theta.back()), ymax = ToDegrees(data.theta.front());
    const std::string xlabel = "time [days]";
    const std::string ylabel = "latitude [deg]";
    const f64 aspectratio = 2;
    PyPlot::Plot({.name = "shift x",
        .z = data.shiftx,
        .xmin = xmin,
        .xmax = xmax,
        .ymin = ymin,
        .ymax = ymax,
        .xlabel = xlabel,
        .ylabel = ylabel,
        .zlabel = "shift x [px]",
        .aspectratio = aspectratio});
    PyPlot::Plot({.name = "omega x",
        .z = data.omegax,
        .xmin = xmin,
        .xmax = xmax,
        .ymin = ymin,
        .ymax = ymax,
        .xlabel = xlabel,
        .ylabel = ylabel,
        .zlabel = "omega x [px]",
        .aspectratio = aspectratio});
    PyPlot::Plot({.name = "shift y",
        .z = data.shifty,
        .xmin = xmin,
        .xmax = xmax,
        .ymin = ymin,
        .ymax = ymax,
        .xlabel = xlabel,
        .ylabel = ylabel,
        .zlabel = "shift y [px]",
        .aspectratio = aspectratio});
    PyPlot::Plot({.name = "omega y",
        .z = data.omegay,
        .xmin = xmin,
        .xmax = xmax,
        .ymin = ymin,
        .ymax = ymax,
        .xlabel = xlabel,
        .ylabel = ylabel,
        .zlabel = "omega y [px]",
        .aspectratio = aspectratio});
  }

  static void SaveOptimizedParameters(const IPC& ipc, const std::string& dataPath, i32 xsizeopt, i32 ysizeopt, i32 popsize)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION;
    std::string path = fmt::format("{}/diffrot_ipcopt.json", dataPath);
    LOG_DEBUG("Saving differential rotation IPC optimization results to {} ...", std::filesystem::weakly_canonical(path).string());

    cv::FileStorage file(path, cv::FileStorage::WRITE);
    file << "xsizeopt" << xsizeopt;
    file << "ysizeopt" << ysizeopt;
    file << "popsize" << popsize;
    file << "IPC" << ipc.Serialize();
  }
};
