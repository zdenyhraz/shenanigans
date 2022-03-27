#pragma once

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

inline void swapbytes(char* input, u32 length)
{
  for (usize i = 0; i < length; i += 2)
    std::swap(input[i], input[i + 1]);
}

class FitsImage
{
public:
  FitsImage() {}

  explicit FitsImage(const std::string& path) : data(loadfits(path)) {}

  void reload(const std::string& path) { data = loadfits(path); }

  const cv::Mat& image() const { return std::get<0>(data); }

  const FitsParams& params() const { return std::get<1>(data); }

private:
  std::tuple<cv::Mat, FitsParams> data;

  static std::tuple<cv::Mat, FitsParams> loadfits(const std::string& path)
  {
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (not file)
      [[unlikely]]
      {
        LOG_ERROR("<loadfits> Cannot load file '{}'- file does not exist dude!", path);
        return std::make_tuple(cv::Mat(), FitsParams());
      }
    else
    {
      // LOG_DEBUG("<loadfits> Loading file '{}'...", path);
      static constexpr i32 lineBytes = 80;
      static constexpr i32 linesMultiplier = 36;
      FitsParams params;
      bool ENDfound = false;
      char cline[lineBytes];
      i32 size = 4096;
      i32 linecnt = 0;
      f64 pixelarcsec = 1;

      while (!file.eof())
      {
        file.read(&cline[0], lineBytes);
        ++linecnt;
        std::string sline(cline);

        if (sline.find("NAXIS1") != std::string::npos)
        {
          usize pos = sline.find("= ");
          std::string snum = sline.substr(pos + 2);
          size = stoi(snum);
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

      cv::Mat mat(size, size, CV_16U);

      file.read(reinterpret_cast<char*>(mat.data), size * size * 2);
      swapbytes(reinterpret_cast<char*>(mat.data), size * size * 2);
      AlignImage(mat);
      params.succload = true;
      params.fitsMidX = size - 1 - params.fitsMidX;

      // cv::normalize(mat, mat, 0, 65535, cv::NORM_MINMAX);

      if constexpr (0)
        DebugValues(mat, params);

      if constexpr (0)
        DebugCircles(mat, params);

      if constexpr (0)
        DebugContours(mat, params);
      else
        params.succParamCorrection = true;

      return std::make_tuple(mat, params);
    }
  }

  static void AlignImage(cv::Mat& mat)
  {
    cv::warpAffine(mat, mat, getRotationMatrix2D(cv::Point2f(mat.cols / 2, mat.rows / 2), 90, 1.0), cv::Size(mat.cols, mat.rows));
    cv::transpose(mat, mat);
  }

  static void DebugValues(const cv::Mat& mat, const FitsParams& params)
  {
    auto [mmin, mmax] = MinMax(mat);
    LOG_INFO("min/max mat {}/{}", mmin, mmax);
    cv::Mat xd = mat.clone();
    cv::cvtColor(xd, xd, cv::COLOR_GRAY2BGR);
    cv::circle(xd, cv::Point(mat.cols / 2, mat.rows / 2), params.R, cv::Scalar(0, 0, 65535), 5);
    Showimg(xd, "xd");
  }

  static void DebugCircles(const cv::Mat& mat, const FitsParams& params)
  {
    cv::Mat img = mat.clone();
    cv::Mat imgc;
    cv::normalize(img, img, 0, 255, cv::NORM_MINMAX);
    img.convertTo(img, CV_8U);
    cv::cvtColor(img, imgc, cv::COLOR_GRAY2BGR);

    // fits
    {
      cv::Point2f center(params.fitsMidX, params.fitsMidY);
      f64 radius = params.R;
      cv::Scalar color(0, 0, 65535);

      // draw the cv::circle center
      cv::circle(imgc, center, 1, color, -1);
      // draw the cv::circle outline
      cv::circle(imgc, center, radius, color, 1, cv::LINE_AA);

      // draw the image center
      cv::circle(imgc, cv::Point2f(mat.cols / 2, mat.rows / 2), 1, cv::Scalar(65535, 0, 0), -1);

      LOG_INFO("Fits center / radius: {} / {}", center, radius);
    }

    // hough
    {
      std::vector<cv::Vec3f> circlesHough;
      HoughCircles(img, circlesHough, cv::HOUGH_GRADIENT, 0.2, img.rows, 200, 100, 1900, 2000);

      cv::Point2f center(circlesHough[0][0], circlesHough[0][1]);
      f64 radius = circlesHough[0][2];
      cv::Scalar color(0, 65535, 0);

      // draw the cv::circle center
      cv::circle(imgc, center, 1, color, -1);
      // draw the cv::circle outline
      cv::circle(imgc, center, radius, color, 1, cv::LINE_AA);

      LOG_INFO("Hough center / radius: {} / {}", center, radius);
    }

    // min enclosing cv::circle
    {
      cv::Mat canny_output;
      Canny(img, canny_output, 50, 200);

      std::vector<std::vector<cv::Point>> contours;
      findContours(canny_output, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

      cv::Point2f center;
      f32 radius;
      cv::Scalar color(65535, 0, 65535);
      minEnclosingCircle(contours[0], center, radius);

      // draw the cv::circle center
      cv::circle(imgc, center, 1, color, -1);
      // draw the cv::circle outline
      cv::circle(imgc, center, radius, color, 1, cv::LINE_AA);

      LOG_INFO("Canny enclosing center / radius: {} / {}", center, radius);
    }

    if constexpr (0) // show
    {
      Showimg(RoiCrop(imgc, 0.5 * 4096, 0.5 * 4096, 100, 100), "circleC");
      Showimg(RoiCrop(imgc, 250, 0.5 * 4096, 500, 500), "circleL");
      Showimg(RoiCrop(imgc, 4096 - 250, 0.5 * 4096, 500, 500), "circleR");
      Showimg(RoiCrop(imgc, 0.5 * 4096, 250, 500, 500), "circleT");
      Showimg(RoiCrop(imgc, 0.5 * 4096, 4096 - 250, 500, 500), "circleB");
    }
  }

  static void DebugContours(const cv::Mat& mat, FitsParams& params)
  {
    // 8-bit input
    cv::Mat img = mat.clone();
    cv::normalize(img, img, 0, 255, cv::NORM_MINMAX);
    img.convertTo(img, CV_8U);

    // canny
    cv::Mat canny_output;
    Canny(img, canny_output, 50, 200);

    // contours
    std::vector<std::vector<cv::Point>> contours;
    findContours(canny_output, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // cv::circle
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

  FitsTime(const std::string& dirpathh, i32 yearr, i32 monthh, i32 dayy, i32 hourr, i32 minutee, i32 secondd)
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

inline void generateFitsDownloadUrlPairs(i32 delta, i32 step, i32 pics, std::string urlmain)
{
  std::ofstream urls("D:\\MainOutput\\Fits_urls\\processedurls_raw.txt", std::ios::out | std::ios::trunc);
  usize posR = urlmain.find("record=");
  usize posN = posR + 7;
  std::string stringcislo = urlmain.substr(posN, 8); // 8mistne cislo
  i32 number = stod(stringcislo);
  urlmain = urlmain.substr(0, posN);
  for (i32 i = 0; i < pics; i++)
  {
    std::string url1 = urlmain + std::to_string(number) + "-" + std::to_string(number);
    number += delta;
    std::string url2 = urlmain + std::to_string(number) + "-" + std::to_string(number);
    urls << url1 << std::endl;
    if (delta > 0)
      urls << url2 << std::endl;

    number += step - delta; // step -delta (step od prvni fotky ne od druhe)
  }
}

inline void generateFitsDownloadUrlSingles(i32 delta, i32 pics, std::string urlmain)
{
  std::ofstream urls("D:\\MainOutput\\Fits_urls\\processedurls_raw.txt", std::ios::out | std::ios::trunc);
  usize posR = urlmain.find("record=");
  usize posN = posR + 7;
  std::string stringcislo = urlmain.substr(posN, 8); // 8mistne cislo
  i32 number = stod(stringcislo);
  urlmain = urlmain.substr(0, posN);
  for (i32 i = 0; i < pics; i++)
  {
    std::string url = urlmain + std::to_string(number) + "-" + std::to_string(number);
    urls << url << std::endl;
    number += delta;
  }
}

inline void checkFitsDownloadUrlPairs(i32 delta, i32 step, i32 pics, std::string urlmain, std::string pathMasterIn)
{
  std::ofstream urls("D:\\MainOutput\\Fits_urls\\processedurls_missing.txt", std::ios::out | std::ios::trunc);
  std::string pathmain = "drms_export.cgi@series=hmi__Ic_45s;record=";
  usize posR = urlmain.find("record=");
  usize posN = posR + 7;
  std::string stringcislo = urlmain.substr(posN, 8); // 8mistne cislo
  i32 number = stod(stringcislo);
  urlmain = urlmain.substr(0, posN);
  for (i32 i = 0; i < pics; i++)
  {
    std::string url1 = urlmain + std::to_string(number) + "-" + std::to_string(number);
    std::string path1 = pathMasterIn + pathmain + std::to_string(number) + "-" + std::to_string(number);
    number += delta;
    std::string url2 = urlmain + std::to_string(number) + "-" + std::to_string(number);
    std::string path2 = pathMasterIn + pathmain + std::to_string(number) + "-" + std::to_string(number);

    std::ifstream stream1(path1, std::ios::binary | std::ios::in);
    if (!stream1)
    {
      LOG_ERROR("File '{}' not found! Adding path to text file ..", path1);
      urls << url1 << std::endl;
    }
    else
    {
      LOG_SUCCESS("File '{}' ok", path1);
    }

    std::ifstream stream2(path2, std::ios::binary | std::ios::in);
    if (!stream2)
    {
      LOG_ERROR("File '{}' not found! Adding path to text file ..", path2);
      urls << url2 << std::endl;
    }
    else
    {
      LOG_SUCCESS("File '{}' ok", path2);
    }

    number += step - delta;
  }
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
