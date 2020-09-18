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

  Mat image() const { return std::get<0>(data); }

  FitsParams params() const { return std::get<1>(data); }

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
      // LOG_DEBUG( "<loadfits> Loading file '{}'...", path );
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

      if (0)
      {
        Mat xd = mat.clone();
        cvtColor(xd, xd, cv::COLOR_GRAY2BGR);
        circle(xd, Point(fitsMid, fitsMid), params.R, Scalar(0, 0, 65535), 5);
        showimg(xd, "xd");
      }

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
