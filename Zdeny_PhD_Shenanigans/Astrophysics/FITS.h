#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Filtering/filtering.h"
#include "Log/logger.h"

using namespace std;
using namespace cv;

//.fits parameters
constexpr int lineBytes = 80;
constexpr int linesMultiplier = 36;
enum class fitsType : char
{
  HMI,
  AIA,
  SECCHI
};

struct FitsParams
{
  double fitsMidX = 0;
  double fitsMidY = 0;
  double R = 0;
  double theta0 = 0;
  bool succload = false;
  bool succParamCorrection = false;
};

inline void swapbytes(char *input, unsigned length)
{
  for (int i = 0; i < length; i += 2)
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

  const Mat &image() const { return std::get<0>(data); }

  const FitsParams &params() const { return std::get<1>(data); }

private:
  std::tuple<Mat, FitsParams> data;

  inline std::tuple<Mat, FitsParams> loadfits(std::string path)
  {
    ifstream streamIN(path, ios::binary | ios::in);
    if (!streamIN)
    {
      LOG_ERROR("<loadfits> Cannot load file '{}'- file does not exist dude!", path);
      return std::make_tuple(Mat(), FitsParams());
    }
    else
    {
      // LOG_DEBUG("<loadfits> Loading file '{}'...", path);
      FitsParams params;
      bool ENDfound = false;
      char cline[lineBytes];
      int fitsSize, fitsMid, fitsSize2;
      int linecnt = 0;
      double pixelarcsec;

      while (!streamIN.eof())
      {
        streamIN.read(&cline[0], lineBytes);
        linecnt++;
        string sline(cline);

        if (sline.find("NAXIS1") != std::string::npos)
        {
          std::size_t pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          fitsSize = stoi(snum);
          fitsMid = fitsSize / 2;
          fitsSize2 = fitsSize * fitsSize;
        }
        else if (sline.find("CRPIX1") != std::string::npos)
        {
          std::size_t pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.fitsMidX = stod(snum) - 1.; // Nasa index od 1
        }
        else if (sline.find("CRPIX2") != std::string::npos)
        {
          std::size_t pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.fitsMidY = stod(snum) - 1.; // Nasa index od 1
        }
        else if (sline.find("CDELT1") != std::string::npos)
        {
          std::size_t pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          pixelarcsec = stod(snum);
        }
        else if (sline.find("RSUN_OBS") != std::string::npos)
        {
          std::size_t pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.R = stod(snum);
        }
        else if (sline.find("CRLT_OBS") != std::string::npos)
        {
          std::size_t pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          params.theta0 = stod(snum) / (360. / 2. / Constants::Pi);
        }
        else if (sline.find("END                        ") != std::string::npos)
        {
          ENDfound = true;
        }

        if (ENDfound && (linecnt % linesMultiplier == 0))
          break;
      }

      params.R /= pixelarcsec;

      // opencv integration
      Mat mat(fitsSize, fitsSize, CV_16UC1);
      streamIN.read((char *)mat.data, fitsSize2 * 2);
      swapbytes((char *)mat.data, fitsSize2 * 2);
      normalize(mat, mat, 0, 65535, NORM_MINMAX);
      warpAffine(mat, mat, getRotationMatrix2D(Point2f(fitsMid, fitsMid), 90, 1.0), cv::Size(fitsSize, fitsSize));
      transpose(mat, mat);
      params.succload = true;
      params.fitsMidX = 4096 - params.fitsMidX; // idk but works

      if constexpr (0) // debug values
      {
        auto [mmin, mmax] = minMaxMat(mat);
        LOG_FATAL("min/max mat {}/{}", mmin, mmax);
        Mat xd = mat.clone();
        cvtColor(xd, xd, cv::COLOR_GRAY2BGR);
        circle(xd, Point(fitsMid, fitsMid), params.R, Scalar(0, 0, 65535), 5);
        showimg(xd, "xd");
      }

      if constexpr (0) // debug circles
      {
        Mat img = mat.clone();
        Mat imgc;
        normalize(img, img, 0, 255, NORM_MINMAX);
        img.convertTo(img, CV_8U);
        cvtColor(img, imgc, COLOR_GRAY2BGR);

        // fits
        {
          Point2f center(params.fitsMidX, params.fitsMidY);
          double radius = params.R;
          Scalar color(0, 0, 65535);

          // draw the circle center
          circle(imgc, center, 1, color, -1);
          // draw the circle outline
          circle(imgc, center, radius, color, 1, LINE_AA);

          // draw the image center
          circle(imgc, Point2f(4096 / 2, 4096 / 2), 1, Scalar(65535, 0, 0), -1);

          LOG_INFO("Fits center / radius: {} / {}", center, radius);
        }

        // hough
        {
          std::vector<Vec3f> circlesHough;
          HoughCircles(img, circlesHough, HOUGH_GRADIENT, 0.2, img.rows, 200, 100, 1900, 2000);

          Point2f center(circlesHough[0][0], circlesHough[0][1]);
          double radius = circlesHough[0][2];
          Scalar color(0, 65535, 0);

          // draw the circle center
          circle(imgc, center, 1, color, -1);
          // draw the circle outline
          circle(imgc, center, radius, color, 1, LINE_AA);

          LOG_INFO("Hough center / radius: {} / {}", center, radius);
        }

        // min enclosing circle
        {
          Mat canny_output;
          Canny(img, canny_output, 50, 200);

          std::vector<std::vector<Point>> contours;
          findContours(canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

          Point2f center;
          float radius;
          Scalar color(65535, 0, 65535);
          minEnclosingCircle(contours[0], center, radius);

          // draw the circle center
          circle(imgc, center, 1, color, -1);
          // draw the circle outline
          circle(imgc, center, radius, color, 1, LINE_AA);

          LOG_INFO("Canny enclosing center / radius: {} / {}", center, radius);
        }

        if constexpr (0) // show
        {
          showimg(roicrop(imgc, 0.5 * 4096, 0.5 * 4096, 100, 100), "circleC" + to_string(rand()));
          showimg(roicrop(imgc, 250, 0.5 * 4096, 500, 500), "circleL" + to_string(rand()));
          showimg(roicrop(imgc, 4096 - 250, 0.5 * 4096, 500, 500), "circleR" + to_string(rand()));
          showimg(roicrop(imgc, 0.5 * 4096, 250, 500, 500), "circleT" + to_string(rand()));
          showimg(roicrop(imgc, 0.5 * 4096, 4096 - 250, 500, 500), "circleB" + to_string(rand()));
        }
      }

      if constexpr (0) // fits midX / midY / R correction by canny + find contours + enclosing circle
      {
        // 8-bit input
        Mat img = mat.clone();
        normalize(img, img, 0, 255, NORM_MINMAX);
        img.convertTo(img, CV_8U);

        // canny
        Mat canny_output;
        Canny(img, canny_output, 50, 200);

        // contours
        std::vector<std::vector<Point>> contours;
        findContours(canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

        // circle
        Point2f center;
        float radius;
        minEnclosingCircle(contours[0], center, radius);

        // only save valid parameters
        if (radius > 1900 && radius < 2000 && center.x > 2030 && center.x < 2070 && center.y > 2030 && center.y < 2070)
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
  int startyear;
  int startmonth;
  int startday;
  int starthour;
  int startminute;
  int startsecond;

  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;

  std::string dirpath;

  std::string yearS;
  std::string monthS;
  std::string dayS;
  std::string hourS;
  std::string minuteS;
  std::string secondS;

  void timeToStringS()
  {
    yearS = to_string(year);
    if (month < 10)
    {
      monthS = "0" + to_string(month);
    }
    else
    {
      monthS = to_string(month);
    }

    if (day < 10)
    {
      dayS = "0" + to_string(day);
    }
    else
    {
      dayS = to_string(day);
    }

    if (hour < 10)
    {
      hourS = "0" + to_string(hour);
    }
    else
    {
      hourS = to_string(hour);
    }

    if (minute < 10)
    {
      minuteS = "0" + to_string(minute);
    }
    else
    {
      minuteS = to_string(minute);
    }

    if (second < 10)
    {
      secondS = "0" + to_string(second);
    }
    else
    {
      secondS = to_string(second);
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

  FitsTime(std::string dirpathh, int yearr, int monthh, int dayy, int hourr, int minutee, int secondd)
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

  void advanceTime(int deltasec)
  {
    second += deltasec;
    int monthdays;
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
    if ((month == 2) && (year % 4 == 0))
      monthdays = 29; // leap year fkn february

    // plus
    if (second >= 60)
    {
      minute += std::floor((double)second / 60.0);
      second %= 60;
    }
    if (minute >= 60)
    {
      hour += std::floor((double)minute / 60.0);
      minute %= 60;
    }
    if (hour >= 24)
    {
      day += std::floor((double)hour / 24.0);
      hour %= 24;
    }
    if (day >= monthdays)
    {
      month += std::floor((double)day / monthdays);
      day %= monthdays;
    }
    // minus
    if (second < 0)
    {
      minute += std::floor((double)second / 60.0);
      second = 60 + second % 60;
    }
    if (minute < 0)
    {
      hour += std::floor((double)minute / 60.0);
      minute = 60 + minute % 60;
    }
    if (hour < 0)
    {
      day += std::floor((double)hour / 24.0);
      hour = 24 + hour % 24;
    }
    if (day < 0)
    {
      month += std::floor((double)day / monthdays);
      day = monthdays + day % monthdays;
    }
  }
};

Mat loadfits(std::string path, FitsParams &params);

void generateFitsDownloadUrlPairs(int delta, int step, int pics, string urlmain);

void generateFitsDownloadUrlSingles(int delta, int pics, string urlmain);

void checkFitsDownloadUrlPairs(int delta, int step, int pics, string urlmain, string pathMasterIn);

void loadImageDebug(Mat &activeimg, double gamaa, bool colorr, double quanBot, double quanTop);

inline Mat loadImage(std::string path)
{
  Mat result;
  if (path.find(".fits") != std::string::npos || path.find(".fts") != std::string::npos)
  {
    result = FitsImage(path).image();
  }
  else
  {
    result = imread(path, IMREAD_ANYDEPTH);
    result.convertTo(result, CV_16U);
    normalize(result, result, 0, 65535, NORM_MINMAX);
  }
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
