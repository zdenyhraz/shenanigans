#include "IPC/IterativePhaseCorrelation.h"

class DifferentialRotation
{
public:
  void Calculate(const IterativePhaseCorrelation& ipc, const std::string& dataPath, i32 idstart) const
  try
  {
    LOG_FUNCTION("DifferentialRotation::Calculate");
    DifferentialRotationData data(xsize, ysize);
    std::atomic<i32> progress = -1;
    const auto ystep = yfov / (ysize - 1);
    const auto tstep = idstep * cadence;
    const auto xpred = 0.;
    const auto wxsize = ipc.GetCols();
    const auto wysize = ipc.GetRows();
    const auto shiftxmax = 1.5 * std::sin(GetPredictedOmega(0, 14.296, -1.847, -2.615) * tstep / Constants::RadPerSecToDegPerDay) * 1936 * std::cos(0.);
    const auto shiftxmin = 0.7 * std::sin(GetPredictedOmega(0, 14.296, -1.847, -2.615) * tstep / Constants::RadPerSecToDegPerDay) * 1936 * std::cos(1.);
    const auto shiftymax = 0.08;
    const auto ids = GenerateIds(idstart);

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
          const auto shiftx = std::clamp(shift.x + xpred, shiftxmin, shiftxmax);
          const auto shifty = std::clamp(shift.y, -shiftymax, shiftymax);
          const auto omegax = std::asin(shiftx / (R * std::cos(theta))) / tstep * Constants::RadPerSecToDegPerDay;
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
        LOG_WARNING("DifferentialRotation::Calculate error: {} - skipping ...", e.what());
        continue;
      }

    if (xsize > 50)
      Save(data, ipc, dataPath);
    Plot(data);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("DifferentialRotation::Calculate error: {}", e.what());
  }

  void LoadAndShow(const std::string& path)
  try
  {
    LOG_FUNCTION("DifferentialRotation::LoadAndShow");
    const auto data = Load(path);
    Plot(data);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("DifferentialRotation::LoadAndShow error: {}", e.what());
  }

  void Optimize(IterativePhaseCorrelation& ipc, i32 populationSize) const
  {
    const auto f = [](const IterativePhaseCorrelation& _ipc)
    {
      // calc diffrot profile
      // calc polyfit
      // calc average polyfit difference from predicted shift
      return 0.;
    };

    ipc.Optimize(f, populationSize);
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
      thetas = cv::Mat::zeros(ysize, xsize, CV_32F);
      shiftsx = cv::Mat::zeros(ysize, xsize, CV_32F);
      shiftsy = cv::Mat::zeros(ysize, xsize, CV_32F);
      omegasx = cv::Mat::zeros(ysize, xsize, CV_32F);
      omegasy = cv::Mat::zeros(ysize, xsize, CV_32F);
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
      ithetamin = std::max(ithetamin, static_cast<f64>(thetas.at<f32>(ysize - 1, x)));
      ithetamax = std::min(ithetamax, static_cast<f64>(thetas.at<f32>(0, x)));
    }

    if (ithetamin == -std::numeric_limits<f64>::infinity() or ithetamax == std::numeric_limits<f64>::infinity())
      throw std::runtime_error("Failed to calculate interpolated thetas");

    LOG_DEBUG("itheta min / max: {:.1f} / {:.1f}", ithetamin * Constants::Rad, ithetamax * Constants::Rad);

    std::vector<f64> ithetas(ysize);
    f64 ithetastep = (ithetamax - ithetamin) / (ysize - 1);
    for (i32 y = 0; y < ysize; ++y)
      ithetas[y] = ithetamax - ithetastep * y;
    return ithetas;
  }

  std::vector<f64> GetTimesInDays(i32 tstep, i32 tstride) const
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

  void Plot(const DifferentialRotationData& rawdata) const
  {
    LOG_FUNCTION("DifferentialRotation::Plot");
    const auto thetas = GetInterpolatedThetas(rawdata.thetas);
    const auto times = GetTimesInDays(idstep * cadence, idstride * cadence);
    auto data = rawdata;

    // apply median blur
    const i32 medsize = 5;
    cv::medianBlur(data.shiftsx, data.shiftsx, medsize);
    cv::medianBlur(data.shiftsy, data.shiftsy, medsize);
    cv::medianBlur(data.omegasx, data.omegasx, medsize);
    cv::medianBlur(data.omegasy, data.omegasy, medsize);

    // apply theta-interpolation
    Interpolate(data.shiftsx, data.thetas, thetas);
    Interpolate(data.shiftsy, data.thetas, thetas);
    Interpolate(data.omegasx, data.thetas, thetas);
    Interpolate(data.omegasy, data.thetas, thetas);

    // fits params
    Plot1D::Set("fits params");
    Plot1D::SetXlabel("time [days]");
    Plot1D::SetYlabel("fits shift [px]");
    Plot1D::SetY2label("theta0 [deg]");
    Plot1D::SetYnames({"fits shift x", "fits shift y"});
    Plot1D::SetY2names({"theta0"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot(times, {data.fshiftsx, data.fshiftsy}, {Constants::Rad * data.theta0s});

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
    Plot1D::SetYnames({"avgomega x", "Derek A. Lamb (2017)", "Howard et al. (1983)"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot(Constants::Rad * thetas, {GetXAverage(data.omegasx), GetPredictedOmegas(thetas, 14.296, -1.847, -2.615), GetPredictedOmegas(thetas, 14.192, -1.70, -2.36)});

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

    char buf[50];
    const auto tm = std::time(nullptr);
    std::strftime(buf, sizeof(buf), "%Y_%b_%d_%H_%M_%S", std::localtime(&tm));
    std::string path = fmt::format("{}/diffrot_{}.json", dataPath, buf);
    LOG_DEBUG("Saving differential rotation results to {} ...", path);
    cv::FileStorage file(path, cv::FileStorage::WRITE);

    // diffrot params
    file << "idstep" << idstep;
    file << "idstride" << idstride;
    file << "xsize" << xsize;
    file << "ysize" << ysize;
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
    file << "fshiftsx" << data.fshiftsx;
    file << "fshiftsy" << data.fshiftsy;
    file << "theta0s" << data.theta0s;
    file << "Rs" << data.Rs;
  }

  DifferentialRotationData Load(const std::string& path)
  {
    LOG_FUNCTION("DifferentialRotation::Load");

    LOG_DEBUG("Loading differential rotation data {}", path);
    cv::FileStorage file(path, cv::FileStorage::READ);
    file["idstep"] >> idstep;
    file["idstride"] >> idstride;
    file["xsize"] >> xsize;
    file["ysize"] >> ysize;
    file["yfov"] >> yfov;
    file["cadence"] >> cadence;

    DifferentialRotationData data(xsize, ysize);
    file["thetas"] >> data.thetas;
    file["shiftsx"] >> data.shiftsx;
    file["shiftsy"] >> data.shiftsy;
    file["omegasx"] >> data.omegasx;
    file["omegasy"] >> data.omegasy;
    file["fshiftsx"] >> data.fshiftsx;
    file["fshiftsy"] >> data.fshiftsy;
    file["theta0s"] >> data.theta0s;
    file["Rs"] >> data.Rs;

    return data;
  }

  i32 idstep = 1;   // id step
  i32 idstride = 0; // id stride
  i32 xsize = 51;   // x size - 2500
  i32 ysize = 851;  // y size - 851
  i32 yfov = 3400;  // y FOV [px]
  i32 cadence = 45; // SDO/HMI cadence [s]
};