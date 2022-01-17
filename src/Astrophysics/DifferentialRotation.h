#include "IPC/IterativePhaseCorrelation.h"

class DifferentialRotation
{
public:
  void Calculate(const std::string& dataPath, i32 idstart) const
  {
    IterativePhaseCorrelation ipc(wysize, wxsize, 0, 1);
    cv::Mat shifts = cv::Mat::zeros(ysize, xsize, CV_32FC2);
    cv::Mat thetas = cv::Mat::zeros(ysize, xsize, CV_32FC1);
    cv::Mat omegas = cv::Mat::zeros(ysize, xsize, CV_32FC2);

    const auto ids = GenerateIds(idstart);
    const auto ystep = yfov / (ysize - 1);
    const auto xpred = 0;
    const auto tstep = idstep * 45;

    for (i32 x = 0; x < xsize; ++x)
    {
      const auto& [id1, id2] = ids[x];
      LOG_DEBUG("[{:.1f}% {} / {}] Calculating diffrot profile {} - {} ...", static_cast<f32>(x) / (xsize - 1) * 100, x + 1, xsize, id1, id2);
      const auto image1 = cv::imread(fmt::format("{}/{}.png", dataPath, id1), cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
      const auto image2 = cv::imread(fmt::format("{}/{}.png", dataPath, id2), cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
      const auto header1 = GetHeader(fmt::format("{}/{}.json", dataPath, id1));
      const auto header2 = GetHeader(fmt::format("{}/{}.json", dataPath, id2));

      for (i32 y = 0; y < ysize; ++y)
      {
        const auto yshift = ystep * (y - ysize / 2);
        const auto theta0 = (header1.theta0 + header2.theta0) / 2;
        const auto R = (header1.R + header2.R) / 2;
        const auto theta = std::asin((ystep * (f64)(ysize / 2 - y)) / R) + theta0;

        auto crop1 = roicrop(image1, header1.xcenter, header1.ycenter + yshift, wxsize, wysize);
        auto crop2 = roicrop(image2, header2.xcenter + xpred, header2.ycenter + yshift, wxsize, wysize);
        const auto shift = ipc.Calculate(std::move(crop1), std::move(crop2));
        const auto omegax = std::asin(shift.x / (R * std::cos(theta))) / tstep * Constants::RadPerSecToDegPerDay;
        const auto omegay = (theta - std::asin(std::sin(theta) - shift.y / R)) / tstep * Constants::RadPerSecToDegPerDay;

        thetas.at<f32>(y, xsize - 1 - x) = theta;
        shifts.at<cv::Vec2f>(y, xsize - 1 - x) = {shift.x + xpred, shift.y};
        omegas.at<cv::Vec2f>(y, xsize - 1 - x) = {static_cast<f32>(omegax), static_cast<f32>(omegay)};
      }
    }
    cv::Mat o[2];
    cv::split(omegas, o);

    Plot2D::Set("omegasX");
    Plot2D::Plot(o[0]);

    Plot2D::Set("omegasY");
    Plot2D::Plot(o[1]);
  }

private:
  struct ImageHeader
  {
    f32 xcenter; // px
    f32 ycenter; // px
    f32 theta0;  // rad
    f32 R;       // px
  };

  ImageHeader GetHeader(const std::string& path) const
  {
    std::ifstream file(path);
    json::json j;
    file >> j;

    ImageHeader header;
    header.xcenter = (j["NAXIS1"].get<f32>()) - (j["CRPIX1"].get<f32>()); // x is flipped, 4095 - fits index from 1
    header.ycenter = j["CRPIX2"].get<f32>() - 1;                          // fits index from 1
    header.theta0 = j["CRLT_OBS"].get<f32>() / Constants::Rad;            // convert from deg to rad
    header.R = j["RSUN_OBS"].get<f32>() / j["CDELT1"].get<f32>();         // arcsec / arcsec per pixel
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

  i32 idstep = 1;    // id step
  i32 idstride = 25; // id stride
  i32 xsize = 2500;  // x size
  i32 ysize = 851;   // y size
  i32 yfov = 3400;   // y FOV
  i32 wxsize = 64;   // window x size
  i32 wysize = 64;   // window y size
};