#include "IPC/IterativePhaseCorrelation.h"

class DifferentialRotation
{
public:
  void Calculate(const std::string& dataPath, i32 idstart) const
  {
    IterativePhaseCorrelation ipc(wysize, wxsize, 0, 1);
    cv::Mat thetas = cv::Mat::zeros(ysize, xsize, CV_64FC1);
    cv::Mat shifts = cv::Mat::zeros(ysize, xsize, CV_64FC2);
    cv::Mat omegas = cv::Mat::zeros(ysize, xsize, CV_64FC2);

    const auto ids = GenerateIds(idstart);
    const auto ystep = yfov / (ysize - 1);
    const auto xpred = 0.;
    const auto tstep = idstep * 45;

    for (i32 x = 0; x < xsize; ++x)
    {
      try
      {
        const auto& [id1, id2] = ids[x];
        LOG_DEBUG("[{:>5.1f}% {} / {}] Calculating diffrot profile {} - {} ...", static_cast<f64>(x) / (xsize - 1) * 100, x + 1, xsize, id1, id2);

        const std::string path1 = fmt::format("{}/{}.png", dataPath, id1);
        const std::string path2 = fmt::format("{}/{}.png", dataPath, id2);
        if (not std::filesystem::exists(path1) or not std::filesystem::exists(path2))
          throw std::runtime_error(fmt::format("Could not load images {} - {}", id1, id2));

        const auto image1 = cv::imread(path1, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
        const auto image2 = cv::imread(path2, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
        const auto header1 = GetHeader(fmt::format("{}/{}.json", dataPath, id1));
        const auto header2 = GetHeader(fmt::format("{}/{}.json", dataPath, id2));

        //#pragma omp parallel for
        for (i32 y = 0; y < ysize; ++y)
        {
          const auto yshift = ystep * (y - ysize / 2);
          const auto theta0 = (header1.theta0 + header2.theta0) / 2;
          const auto R = (header1.R + header2.R) / 2;
          const auto theta = std::asin((ystep * (f64)(ysize / 2 - y)) / R) + theta0;

          auto crop1 = roicrop(image1, header1.xcenter, header1.ycenter + yshift, wxsize, wysize);
          auto crop2 = roicrop(image2, header2.xcenter + xpred, header2.ycenter + yshift, wxsize, wysize);
          const auto shift = ipc.Calculate(std::move(crop1), std::move(crop2));

          const auto shiftx = std::clamp(shift.x + xpred, 0., 1.);
          const auto shifty = std::clamp(shift.y, -0.1, 0.1);
          const auto omegax = std::asin(shiftx / (R * std::cos(theta))) / tstep * Constants::RadPerSecToDegPerDay;
          const auto omegay = (theta - std::asin(std::sin(theta) - shifty / R)) / tstep * Constants::RadPerSecToDegPerDay;

          // LOG_DEBUG("s: {}, ox: {:.1f}, oy: {:.1f}", shift, omegax, omegay);

          thetas.at<f64>(y, xsize - 1 - x) = theta;
          shifts.at<cv::Vec2d>(y, xsize - 1 - x) = cv::Vec2d(shiftx, shifty);
          omegas.at<cv::Vec2d>(y, xsize - 1 - x) = cv::Vec2d(omegax, omegay);
        }
      }
      catch (const std::exception& e)
      {
        LOG_WARNING("DifferentialRotation::Calculate error: {} - skipping ...", e.what());
        continue;
      }
    }

    cv::Mat _shifts[2];
    cv::split(shifts, _shifts);
    Plot2D::Set("shiftsx");
    Plot2D::Plot(_shifts[0]);
    Plot2D::Set("shiftsy");
    Plot2D::Plot(_shifts[1]);

    // theta-interpolated values
    const auto ithetas = GetInterpolatedThetas(thetas);
    cv::Mat ishifts = cv::Mat::zeros(ysize, xsize, CV_64FC2);
    cv::Mat iomegas = cv::Mat::zeros(ysize, xsize, CV_64FC2);
  }

private:
  struct ImageHeader
  {
    f64 xcenter; // px
    f64 ycenter; // px
    f64 theta0;  // rad
    f64 R;       // px
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
      id += idstride;
    }
    return ids;
  }

  std::vector<f64> GetInterpolatedThetas(const cv::Mat& thetas) const
  {
    f64 ithetamin = -1; // highest S latitude
    f64 ithetamax = 1;  // lowest N latitude
    for (i32 x = 0; x < xsize; ++x)
    {
      ithetamin = std::max(ithetamin, thetas.at<f64>(ysize - 1, x));
      ithetamax = std::min(ithetamax, thetas.at<f64>(0, x));
    }
    LOG_DEBUG("iTheta min / max: {} / {}", ithetamin, ithetamax);

    std::vector<f64> ithetas(ysize);
    f64 ithetastep = (ithetamax - ithetamin) / (ysize - 1);
    for (i32 y = 0; y < ysize; ++y)
      ithetas[y] = ithetamax - ithetastep * y;
    return ithetas;
  }

  i32 idstep = 1;    // id step
  i32 idstride = 25; // id stride
  i32 xsize = 30;    // 2500;  // x size
  i32 ysize = 31;    // 851;   // y size
  i32 yfov = 3400;   // y FOV
  i32 wxsize = 64;   // window x size
  i32 wysize = 64;   // window y size
};