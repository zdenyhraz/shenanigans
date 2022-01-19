#include "IPC/IterativePhaseCorrelation.h"

class DifferentialRotation
{
public:
  void Calculate(const std::string& dataPath, i32 idstart) const
  try
  {
    IterativePhaseCorrelation ipc(wysize, wxsize, 0, 0.6);
    DifferentialRotationData data(xsize, ysize);
    std::atomic<i32> progress = -1;

    const auto ids = GenerateIds(idstart);
    const auto ystep = yfov / (ysize - 1);
    const auto tstep = idstep * cadence;
    const auto xpred = 0.;

#pragma omp parallel for
    for (i32 x = 0; x < xsize; ++x)
      try
      {
        const auto logprogress = static_cast<f64>(progress.fetch_add(1, std::memory_order_relaxed)) + 1.;
        const auto& [id1, id2] = ids[x];
        const std::string path1 = fmt::format("{}/{}.png", dataPath, id1);
        const std::string path2 = fmt::format("{}/{}.png", dataPath, id2);
        if (std::filesystem::exists(path1) and std::filesystem::exists(path2)) [[likely]]
        {
          LOG_DEBUG("[{:>3.0f}%: {} / {}] Calculating diffrot profile {} - {} ...", logprogress / (xsize - 1) * 100, logprogress + 1, xsize, id1, id2);
        }
        else [[unlikely]]
        {
          LOG_WARNING("[{:>3.0f}%: {} / {}] Could not load images {} - {}, skipping ...", logprogress / (xsize - 1) * 100, logprogress + 1, xsize, id1, id2);
          continue;
        }

        const auto image1 = cv::imread(path1, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
        const auto image2 = cv::imread(path2, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
        const auto header1 = GetHeader(fmt::format("{}/{}.json", dataPath, id1));
        const auto header2 = GetHeader(fmt::format("{}/{}.json", dataPath, id2));
        const auto theta0 = (header1.theta0 + header2.theta0) / 2;
        const auto R = (header1.R + header2.R) / 2;
        const auto xindex = xsize - 1 - x;
        data.fshiftsx[xindex] = header2.xcenter - header1.xcenter;
        data.fshiftsy[xindex] = header2.ycenter - header1.ycenter;
        data.theta0s[xindex] = theta0;
        data.Rs[xindex] = R;

        for (i32 y = 0; y < ysize; ++y)
        {
          const auto yshift = ystep * (y - ysize / 2);
          const auto theta = std::asin((f64)(-yshift) / R) + theta0;
          auto crop1 = roicrop(image1, header1.xcenter, header1.ycenter + yshift, wxsize, wysize);
          auto crop2 = roicrop(image2, header2.xcenter + xpred, header2.ycenter + yshift, wxsize, wysize);
          const auto shift = ipc.Calculate(std::move(crop1), std::move(crop2));
          const auto shiftx = std::clamp(shift.x + xpred, 0., 1.);
          const auto shifty = std::clamp(shift.y, -0.1, 0.1);
          const auto omegax = std::asin(shiftx / (R * std::cos(theta))) / tstep * Constants::RadPerSecToDegPerDay;
          const auto omegay = (theta - std::asin(std::sin(theta) - shifty / R)) / tstep * Constants::RadPerSecToDegPerDay;

          data.thetas.at<f64>(y, xindex) = theta;
          data.shiftsx.at<f64>(y, xindex) = shiftx;
          data.shiftsy.at<f64>(y, xindex) = shifty;
          data.omegasx.at<f64>(y, xindex) = omegax;
          data.omegasy.at<f64>(y, xindex) = omegay;
        }
      }
      catch (const std::exception& e)
      {
        LOG_WARNING("DifferentialRotation::Calculate error: {} - skipping ...", e.what());
        continue;
      }

    Plot(data);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("DifferentialRotation::Calculate error: {}", e.what());
  }

private:
  struct ImageHeader
  {
    f64 xcenter; // [px]
    f64 ycenter; // [px]
    f64 theta0;  // [rad]
    f64 R;       // [px]
  };

  struct DifferentialRotationData
  {
    DifferentialRotationData(i32 xsize, i32 ysize)
    {
      thetas = cv::Mat::zeros(ysize, xsize, CV_64F);
      shiftsx = cv::Mat::zeros(ysize, xsize, CV_64F);
      shiftsy = cv::Mat::zeros(ysize, xsize, CV_64F);
      omegasx = cv::Mat::zeros(ysize, xsize, CV_64F);
      omegasy = cv::Mat::zeros(ysize, xsize, CV_64F);
      fshiftsx.resize(xsize);
      fshiftsy.resize(xsize);
      theta0s.resize(xsize);
      Rs.resize(xsize);
    }

    cv::Mat thetas, shiftsx, shiftsy, omegasx, omegasy;
    std::vector<f64> fshiftsx, fshiftsy, theta0s, Rs;
  };

  ImageHeader GetHeader(const std::string& path) const
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

  std::vector<std::tuple<i32, i32>> GenerateIds(i32 idstart) const
  {
    std::vector<std::tuple<i32, i32>> ids(xsize);
    i32 id = idstart;
    for (i32 x = 0; x < xsize; ++x)
    {
      ids[x] = {id, id + idstep};
      id += idstride != 0 ? idstride : idstep;
    }
    return ids;
  }

  std::vector<f64> GetInterpolatedThetas(const cv::Mat& thetas) const
  {
    f64 ithetamin = -std::numeric_limits<f64>::infinity(); // highest S latitude
    f64 ithetamax = std::numeric_limits<f64>::infinity();  // lowest N latitude
    for (i32 x = 0; x < xsize; ++x)
    {
      ithetamin = std::max(ithetamin, thetas.at<f64>(ysize - 1, x));
      ithetamax = std::min(ithetamax, thetas.at<f64>(0, x));
    }

    if (ithetamin == -std::numeric_limits<f64>::infinity() or ithetamax == std::numeric_limits<f64>::infinity())
      throw std::runtime_error("Failed to calculate interpolated thetas");

    LOG_DEBUG("iTheta min / max: {:.1f} / {:.1f}", ithetamin * Constants::Rad, ithetamax * Constants::Rad);

    std::vector<f64> ithetas(ysize);
    f64 ithetastep = (ithetamax - ithetamin) / (ysize - 1);
    for (i32 y = 0; y < ysize; ++y)
      ithetas[y] = ithetamax - ithetastep * y;
    return ithetas;
  }

  std::vector<f64> GetTimesInDays(i32 tstep, i32 tstride) const
  {
    std::vector<f64> times(xsize); // [days]
    f64 time = 0;                  // [s]
    for (i32 i = 0; i < xsize; ++i)
    {
      times[i] = time / 60 / 60 / 24;
      time += tstride != 0 ? tstride : tstep;
    }
    return times;
  }

  static i32 FindFirstLowerIndex(const cv::Mat& thetas, i32 x, f64 itheta)
  {
    for (i32 y = 0; y < thetas.rows; ++y)
      if (itheta <= thetas.at<f64>(y, x))
        return y;

    throw std::runtime_error(fmt::format("Could not find theta higher than {:.1f}", itheta));
  }

  static cv::Mat Interpolate(const cv::Mat& vals, const cv::Mat& thetas, const std::vector<f64>& ithetas)
  {
    cv::Mat ivals = cv::Mat::zeros(vals.rows, vals.cols, CV_64F);
    for (i32 x = 0; x < vals.cols; ++x)
    {
      for (i32 y = 0; y < vals.rows; ++y)
      {
        const auto theta = ithetas[y];
        const auto iL = FindFirstLowerIndex(thetas, x, theta);
        const auto iH = iL + 1;
        const auto thetaL = thetas.at<f64>(iL, x);
        const auto thetaH = thetas.at<f64>(iH, x);
        const auto valL = vals.at<f64>(iL, x);
        const auto valH = vals.at<f64>(iH, x);
        const f64 t = (theta - thetaL) / (thetaH - thetaL);
        ivals.at<f64>(y, x) = std::lerp(valL, valH, t);
      }
    }
    return ivals;
  }

  void Plot(const DifferentialRotationData& data) const
  {
    // theta-interpolated values
    const auto thetas = GetInterpolatedThetas(data.thetas);
    const auto times = GetTimesInDays(idstep * cadence, idstride * cadence);
    // cv::Mat ishifts = cv::Mat::zeros(ysize, xsize, CV_64FC2);
    // cv::Mat iomegas = cv::Mat::zeros(ysize, xsize, CV_64FC2);

    Plot1D::Set("fits params");
    Plot1D::SetXlabel("time [days]");
    Plot1D::SetYlabel("fits shift [px]");
    Plot1D::SetY2label("theta0 [deg]");
    Plot1D::SetYnames({"fits shift x", "fits shift y"});
    Plot1D::SetY2names({"theta0"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot(times, {data.fshiftsx, data.fshiftsy}, {Constants::Rad * data.theta0s});

    Plot2D::Set("shiftsx");
    Plot2D::ShowAxisLabels(true);
    Plot2D::SetXmin(times.front());
    Plot2D::SetXmax(times.back());
    Plot2D::SetYmin(thetas.back() * Constants::Rad);
    Plot2D::SetYmax(thetas.front() * Constants::Rad);
    Plot2D::SetXlabel("time [days]");
    Plot2D::SetYlabel("latitude [deg]");
    Plot2D::SetZlabel("shiftsx [px]");
    Plot2D::Plot(data.shiftsx);

    Plot2D::Set("ishiftsx");
    Plot2D::ShowAxisLabels(true);
    Plot2D::SetXmin(times.front());
    Plot2D::SetXmax(times.back());
    Plot2D::SetYmin(thetas.back() * Constants::Rad);
    Plot2D::SetYmax(thetas.front() * Constants::Rad);
    Plot2D::SetXlabel("time [days]");
    Plot2D::SetYlabel("latitude [deg]");
    Plot2D::SetZlabel("ishiftsx [px]");
    Plot2D::Plot(Interpolate(data.shiftsx, data.thetas, thetas));
  }

  static constexpr i32 cadence = 45; // SDO/HMI cadence [s]
  i32 idstep = 1;                    // id step
  i32 idstride = 0;                  // id stride
  i32 xsize = 50;                    // x size - 2500
  i32 ysize = 51;                    // y size - 851
  i32 yfov = 3400;                   // y FOV [px]
  i32 wxsize = 64;                   // window x size [px]
  i32 wysize = 64;                   // window y size [px]
};