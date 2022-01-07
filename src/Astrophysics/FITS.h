#pragma once
#include "Utils/functionsBaseSTL.h"
#include "UtilsCV/functionsBaseCV.h"
#include "Filtering/filtering.h"
#include "Log/logger.h"
#include "UtilsCV/showsave.h"

//.fits parameters
constexpr i32 lineBytes = 80;
constexpr i32 linesMultiplier = 36;
enum class fitsType : char
{
  HMI,
  AIA,
  SECCHI
};

struct FitsParams
{
  f64 fitsMidX = 0;
  f64 fitsMidY = 0;
  f64 R = 0;
  f64 theta0 = 0;
  bool succload = false;
  bool succParamCorrection = false;
};

inline void swapbytes(char* input, unsigned length)
{
  for (usize i = 0; i < length; i += 2)
  {
    char temp = std::move(input[i]);
    input[i] = std::move(input[i + 1]);
    input[i + 1] = std::move(temp);
  }
}

class FitsImage
{
public:
  FitsImage() {}

  FitsImage(std::string path) { data = loadfits(path); }

  void reload(std::string path) { data = loadfits(path); }

  const cv::Mat& image() const { return std::get<0>(data); }

  const FitsParams& params() const { return std::get<1>(data); }

private:
  std::tuple<cv::Mat, FitsParams> data;

  inline std::tuple<cv::Mat, FitsParams> loadfits(std::string path)
  {
    std::ifstream streamIN(path, std::ios::binary | std::ios::in);
    if (!streamIN)
    {
      LOG_ERROR("<loadfits> Cannot load file '{}'- file does not exist dude!", path);
      return std::make_tuple(cv::Mat(), FitsParams());
    }
    else
    {
      // LOG_DEBUG("<loadfits> Loading file '{}'...", path);
      FitsParams params;
      bool ENDfound = false;
      char cline[lineBytes];
      i32 fitsSize = 4096, fitsMid = 4096 / 2, fitsSize2 = fitsSize * fitsSize;
      i32 linecnt = 0;
      f64 pixelarcsec = 1;

      while (!streamIN.eof())
      {
        streamIN.read(&cline[0], lineBytes);
        linecnt++;
        std::string sline(cline);

        if (sline.find("NAXIS1") != std::string::npos)
        {
          usize pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          fitsSize = stoi(snum);
          fitsMid = fitsSize / 2;
          fitsSize2 = fitsSize * fitsSize;
        }
        else if (sline.find("CRPIX1") != std::string::npos)
        {
          usize pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.fitsMidX = stod(snum) - 1.; // Nasa index od 1
        }
        else if (sline.find("CRPIX2") != std::string::npos)
        {
          usize pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.fitsMidY = stod(snum) - 1.; // Nasa index od 1
        }
        else if (sline.find("CDELT1") != std::string::npos)
        {
          usize pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          pixelarcsec = stod(snum);
        }
        else if (sline.find("RSUN_OBS") != std::string::npos)
        {
          usize pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.R = stod(snum);
        }
        else if (sline.find("CRLT_OBS") != std::string::npos)
        {
          usize pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.theta0 = stod(snum) / (360. / 2. / Constants::Pi);
        }
        else if (sline.find("END                        ") != std::string::npos)
        {
          ENDfound = true;
        }

        if (ENDfound and (linecnt % linesMultiplier == 0))
          break;
      }

      params.R /= pixelarcsec;

      // opencv integration
      cv::Mat mat(fitsSize, fitsSize, CV_16UC1);
      streamIN.read((char*)mat.data, fitsSize2 * 2);
      swapbytes((char*)mat.data, fitsSize2 * 2);
      normalize(mat, mat, 0, 65535, cv::NORM_MINMAX);
      warpAffine(mat, mat, getRotationMatrix2D(cv::Point2f(fitsMid, fitsMid), 90, 1.0), cv::Size(fitsSize, fitsSize));
      transpose(mat, mat);
      params.succload = true;
      params.fitsMidX = 4096 - params.fitsMidX; // idk but works

      if constexpr (0) // debug values
      {
        auto [mmin, mmax] = minMaxMat(mat);
        LOG_INFO("min/max mat {}/{}", mmin, mmax);
        cv::Mat xd = mat.clone();
        cvtColor(xd, xd, cv::COLOR_GRAY2BGR);
        circle(xd, cv::Point(fitsMid, fitsMid), params.R, cv::Scalar(0, 0, 65535), 5);
        showimg(xd, "xd");
      }

      if constexpr (0) // debug circles
      {
        cv::Mat img = mat.clone();
        cv::Mat imgc;
        normalize(img, img, 0, 255, cv::NORM_MINMAX);
        img.convertTo(img, CV_8U);
        cvtColor(img, imgc, cv::COLOR_GRAY2BGR);

        // fits
        {
          cv::Point2f center(params.fitsMidX, params.fitsMidY);
          f64 radius = params.R;
          cv::Scalar color(0, 0, 65535);

          // draw the circle center
          circle(imgc, center, 1, color, -1);
          // draw the circle outline
          circle(imgc, center, radius, color, 1, cv::LINE_AA);

          // draw the image center
          circle(imgc, cv::Point2f(4096 / 2, 4096 / 2), 1, cv::Scalar(65535, 0, 0), -1);

          LOG_INFO("Fits center / radius: {} / {}", center, radius);
        }

        // hough
        {
          std::vector<cv::Vec3f> circlesHough;
          HoughCircles(img, circlesHough, cv::HOUGH_GRADIENT, 0.2, img.rows, 200, 100, 1900, 2000);

          cv::Point2f center(circlesHough[0][0], circlesHough[0][1]);
          f64 radius = circlesHough[0][2];
          cv::Scalar color(0, 65535, 0);

          // draw the circle center
          circle(imgc, center, 1, color, -1);
          // draw the circle outline
          circle(imgc, center, radius, color, 1, cv::LINE_AA);

          LOG_INFO("Hough center / radius: {} / {}", center, radius);
        }

        // min enclosing circle
        {
          cv::Mat canny_output;
          Canny(img, canny_output, 50, 200);

          std::vector<std::vector<cv::Point>> contours;
          findContours(canny_output, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

          cv::Point2f center;
          f32 radius;
          cv::Scalar color(65535, 0, 65535);
          minEnclosingCircle(contours[0], center, radius);

          // draw the circle center
          circle(imgc, center, 1, color, -1);
          // draw the circle outline
          circle(imgc, center, radius, color, 1, cv::LINE_AA);

          LOG_INFO("Canny enclosing center / radius: {} / {}", center, radius);
        }

        if constexpr (0) // show
        {
          showimg(roicrop(imgc, 0.5 * 4096, 0.5 * 4096, 100, 100), "circleC" + std::to_string(rand()));
          showimg(roicrop(imgc, 250, 0.5 * 4096, 500, 500), "circleL" + std::to_string(rand()));
          showimg(roicrop(imgc, 4096 - 250, 0.5 * 4096, 500, 500), "circleR" + std::to_string(rand()));
          showimg(roicrop(imgc, 0.5 * 4096, 250, 500, 500), "circleT" + std::to_string(rand()));
          showimg(roicrop(imgc, 0.5 * 4096, 4096 - 250, 500, 500), "circleB" + std::to_string(rand()));
        }
      }

      if constexpr (0) // fits midX / midY / R correction by canny + find contours + enclosing circle
      {
        // 8-bit input
        cv::Mat img = mat.clone();
        normalize(img, img, 0, 255, cv::NORM_MINMAX);
        img.convertTo(img, CV_8U);

        // canny
        cv::Mat canny_output;
        Canny(img, canny_output, 50, 200);

        // contours
        std::vector<std::vector<cv::Point>> contours;
        findContours(canny_output, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        // circle
        cv::Point2f center;
        f32 radius;
        minEnclosingCircle(contours[0], center, radius);

        // only save valid parameters
        if (radius > 1900 and radius < 2000 and center.x > 2030 and center.x < 2070 and center.y > 2030 and center.y < 2070)
        {
          params.fitsMidX = center.x;
          params.fitsMidY = center.y;
          params.R = radius;
          params.succParamCorrection = true;
        }
        else
          LOG_ERROR("<loadfits> Subpixel parameter correction invalid - center / radius: {} / {}", center, radius);
      }
      else
        params.succParamCorrection = true;

      return std::make_tuple(mat, params);
    }
  }
};

struct FitsTime
{
private:
  i32 startyear;
  i32 startmonth;
  i32 startday;
  i32 starthour;
  i32 startminute;
  i32 startsecond;

  i32 year;
  i32 month;
  i32 day;
  i32 hour;
  i32 minute;
  i32 second;

  std::string dirpath;

  std::string yearS;
  std::string monthS;
  std::string dayS;
  std::string hourS;
  std::string minuteS;
  std::string secondS;

  void timeToStringS()
  {
    yearS = std::to_string(year);
    if (month < 10)
    {
      monthS = "0" + std::to_string(month);
    }
    else
    {
      monthS = std::to_string(month);
    }

    if (day < 10)
    {
      dayS = "0" + std::to_string(day);
    }
    else
    {
      dayS = std::to_string(day);
    }

    if (hour < 10)
    {
      hourS = "0" + std::to_string(hour);
    }
    else
    {
      hourS = std::to_string(hour);
    }

    if (minute < 10)
    {
      minuteS = "0" + std::to_string(minute);
    }
    else
    {
      minuteS = std::to_string(minute);
    }

    if (second < 10)
    {
      secondS = "0" + std::to_string(second);
    }
    else
    {
      secondS = std::to_string(second);
    }
  }

public:
  void resetTime()
  {
    year = startyear;
    month = startmonth;
    day = startday;
    hour = starthour;
    minute = startminute;
    second = startsecond;
  }

  FitsTime(std::string dirpathh, i32 yearr, i32 monthh, i32 dayy, i32 hourr, i32 minutee, i32 secondd)
  {
    dirpath = dirpathh;
    startyear = yearr;
    startmonth = monthh;
    startday = dayy;
    starthour = hourr;
    startminute = minutee;
    startsecond = secondd;
    resetTime();
    advanceTime(0);
    timeToStringS();
  }

  std::string path()
  {
    timeToStringS();
    return dirpath + yearS + "_" + monthS + "_" + dayS + "__" + hourS + "_" + minuteS + "_" + secondS + "__CONT.fits";
  }

  void advanceTime(i32 deltasec)
  {
    second += deltasec;
    i32 monthdays;
    if (month <= 7) // first seven months
    {
      if (month % 2 == 0)
        monthdays = 30;
      else
        monthdays = 31;
    }
    else // last 5 months
    {
      if (month % 2 == 0)
        monthdays = 31;
      else
        monthdays = 30;
    }
    if (month == 2)
      monthdays = 28; // february
    if ((month == 2) and (year % 4 == 0))
      monthdays = 29; // leap year fkn february

    // plus
    if (second >= 60)
    {
      minute += std::floor((f64)second / 60.0);
      second %= 60;
    }
    if (minute >= 60)
    {
      hour += std::floor((f64)minute / 60.0);
      minute %= 60;
    }
    if (hour >= 24)
    {
      day += std::floor((f64)hour / 24.0);
      hour %= 24;
    }
    if (day >= monthdays)
    {
      month += std::floor((f64)day / monthdays);
      day %= monthdays;
    }
    // minus
    if (second < 0)
    {
      minute += std::floor((f64)second / 60.0);
      second = 60 + second % 60;
    }
    if (minute < 0)
    {
      hour += std::floor((f64)minute / 60.0);
      minute = 60 + minute % 60;
    }
    if (hour < 0)
    {
      day += std::floor((f64)hour / 24.0);
      hour = 24 + hour % 24;
    }
    if (day < 0)
    {
      month += std::floor((f64)day / monthdays);
      day = monthdays + day % monthdays;
    }
  }
};

cv::Mat loadfits(std::string path, FitsParams& params);

void generateFitsDownloadUrlPairs(i32 delta, i32 step, i32 pics, std::string urlmain);

void generateFitsDownloadUrlSingles(i32 delta, i32 pics, std::string urlmain);

void checkFitsDownloadUrlPairs(i32 delta, i32 step, i32 pics, std::string urlmain, std::string pathMasterIn);

void loadImageDebug(cv::Mat& activeimg, f64 gamaa, bool colorr, f64 quanBot, f64 quanTop);

inline cv::Mat loadImage(const std::string& path)
{
  if (path.find(".fits") != std::string::npos or path.find(".fts") != std::string::npos)
    return FitsImage(path).image();

  cv::Mat result = cv::imread(path, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
  if (result.empty())
    throw std::runtime_error(fmt::format("Could not load image '{}'", path));
  result.convertTo(result, CV_32F);
  normalize(result, result, 0, 1, cv::NORM_MINMAX);
  return result;
}

inline void fitsDownloaderImpl()
{
  std::string urlmain = "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122";
  // generateFitsDownloadUrlSingles( 1, 2000, urlmain );
  generateFitsDownloadUrlPairs(1, 25, 2500, urlmain);
  LOG_INFO("Fits download urls created");
}

inline void fitsDownloadCheckerImpl()
{
  // std::string urlmain = "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122";
  std::string urlmain = "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18982272-18982272";
  std::string path = "D:\\SDOpics\\Calm2020stride25plus\\";
  checkFitsDownloadUrlPairs(1, 25, 535, urlmain, path);
  LOG_INFO("Fits download urls checked");
}
