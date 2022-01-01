
#include "FITS.h"

cv::Mat loadfits(std::string path, FitsParams& params)
{
  std::ifstream streamIN(path, std::ios::binary | std::ios::in);
  if (!streamIN)
  {
    std::cout << "<loadfits> Cannot load file '" << path << "'- file does not exist dude!" << std::endl;
    cv::Mat shit;
    params.succload = false;
    return shit;
  }
  else
  {
    bool ENDfound = false;
    char lajnaText[lineBytes];
    size_t fitsSize = 4096;
    size_t fitsMid = fitsSize / 2;
    size_t fitsSize2 = fitsSize * fitsSize;
    size_t lajny = 0;
    double pixelarcsec = 1;

    while (!streamIN.eof())
    {
      streamIN.read(&lajnaText[0], lineBytes);
      lajny++;
      std::string lajnaString(lajnaText);

      if (lajnaString.find("NAXIS1") != std::string::npos)
      {
        std::size_t pos = lajnaString.find("= ");
        std::string stringcislo = lajnaString.substr(pos + 2);
        fitsSize = stoi(stringcislo);
        fitsMid = fitsSize / 2;
        fitsSize2 = fitsSize * fitsSize;
      }
      else if (lajnaString.find("CRPIX1") != std::string::npos)
      {
        std::size_t pos = lajnaString.find("= ");
        std::string stringcislo = lajnaString.substr(pos + 2);
        params.fitsMidX = stod(stringcislo) - 1.; // Nasa index od 1
      }
      else if (lajnaString.find("CRPIX2") != std::string::npos)
      {
        std::size_t pos = lajnaString.find("= ");
        std::string stringcislo = lajnaString.substr(pos + 2);
        params.fitsMidY = stod(stringcislo) - 1.; // Nasa index od 1
      }
      else if (lajnaString.find("CDELT1") != std::string::npos)
      {
        std::size_t pos = lajnaString.find("= ");
        std::string stringcislo = lajnaString.substr(pos + 2);
        pixelarcsec = stod(stringcislo);
      }
      else if (lajnaString.find("RSUN_OBS") != std::string::npos)
      {
        std::size_t pos = lajnaString.find("= ");
        std::string stringcislo = lajnaString.substr(pos + 2);
        params.R = stod(stringcislo);
      }
      else if (lajnaString.find("CRLT_OBS") != std::string::npos)
      {
        std::size_t pos = lajnaString.find("= ");
        std::string stringcislo = lajnaString.substr(pos + 2);
        params.theta0 = stod(stringcislo) / (360. / 2. / Constants::Pi);
      }
      else if (lajnaString.find("END                        ") != std::string::npos)
      {
        ENDfound = true;
      }

      if (ENDfound and (lajny % linesMultiplier == 0))
        break;
    }
    params.R /= pixelarcsec;
    std::vector<char> celyobraz_8(fitsSize2 * 2);
    streamIN.read(celyobraz_8.data(), fitsSize2 * 2);
    swapbytes(celyobraz_8.data(), fitsSize2 * 2);
    short* P_shortArray = (short*)(celyobraz_8.data());
    ushort* P_ushortArray = (ushort*)(celyobraz_8.data());

    if (1) // new korekce
    {
      std::vector<int> pixely(fitsSize2, 0);
      //#pragma omp parallel for
      for (size_t i = 0; i < fitsSize2; i++)
      {
        // P_shortArray[i] -= DATAMIN;
        int px = (int)(P_shortArray[i]);
        px += 32768;
        pixely[i] = px;
        if (0 and (px > 65535))
        {
          std::cout << "what? pixel out of 16bit range.";
          std::cin.ignore();
        }
        P_ushortArray[i] = px;
      }
      // cout << "min/max pixel: " << vectorMin(pixely) << " / " << vectorMax(pixely) << std::endl;
    }

    // opencv integration
    cv::Mat mat = cv::Mat(fitsSize, fitsSize, CV_16UC1, celyobraz_8.data(), cv::Mat::AUTO_STEP).clone();
    normalize(mat, mat, 0, 65535, cv::NORM_MINMAX);
    cv::Point2f pt(fitsMid, fitsMid);
    cv::Mat r = getRotationMatrix2D(pt, 90, 1.0);
    warpAffine(mat, mat, r, cv::Size(fitsSize, fitsSize));
    transpose(mat, mat);
    params.succload = true;
    return mat;
  }
}

void generateFitsDownloadUrlPairs(int delta, int step, int pics, std::string urlmain)
{
  std::ofstream urls("D:\\MainOutput\\Fits_urls\\processedurls_raw.txt", std::ios::out | std::ios::trunc);
  std::size_t posR = urlmain.find("record=");
  std::size_t posN = posR + 7;
  std::string stringcislo = urlmain.substr(posN, 8); // 8mistne cislo
  int number = stod(stringcislo);
  urlmain = urlmain.substr(0, posN);
  for (int i = 0; i < pics; i++)
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

void generateFitsDownloadUrlSingles(int delta, int pics, std::string urlmain)
{
  std::ofstream urls("D:\\MainOutput\\Fits_urls\\processedurls_raw.txt", std::ios::out | std::ios::trunc);
  size_t posR = urlmain.find("record=");
  size_t posN = posR + 7;
  std::string stringcislo = urlmain.substr(posN, 8); // 8mistne cislo
  int number = stod(stringcislo);
  urlmain = urlmain.substr(0, posN);
  for (int i = 0; i < pics; i++)
  {
    std::string url = urlmain + std::to_string(number) + "-" + std::to_string(number);
    urls << url << std::endl;
    number += delta;
  }
}

void checkFitsDownloadUrlPairs(int delta, int step, int pics, std::string urlmain, std::string pathMasterIn)
{
  std::ofstream urls("D:\\MainOutput\\Fits_urls\\processedurls_missing.txt", std::ios::out | std::ios::trunc);
  std::string pathmain = "drms_export.cgi@series=hmi__Ic_45s;record=";
  size_t posR = urlmain.find("record=");
  size_t posN = posR + 7;
  std::string stringcislo = urlmain.substr(posN, 8); // 8mistne cislo
  int number = stod(stringcislo);
  urlmain = urlmain.substr(0, posN);
  for (int i = 0; i < pics; i++)
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

void loadImageDebug(cv::Mat& activeimg, double gamaa, bool colorr, double quanBot, double quanTop)
{
  std::string path = "xd";
  if (path.find(".fits") != std::string::npos or path.find(".fts") != std::string::npos)
  {
    std::cout << "loading a fits file.." << std::endl;
    FitsParams params;
    activeimg = gammaCorrect(loadfits(path, params), gamaa);
    std::cout << "fitsMidX: " << params.fitsMidX << std::endl;
    std::cout << "fitsMidY: " << params.fitsMidY << std::endl;
    std::cout << "R: " << params.R << std::endl;
    std::cout << "theta0: " << params.theta0 << std::endl;
  }
  else
  {
    std::cout << "loading a picture.." << std::endl;
    if (colorr)
      activeimg = cv::imread(path, cv::IMREAD_COLOR);
    else
      activeimg = cv::imread(path, cv::IMREAD_ANYDEPTH); // cv::IMREAD_ANYDEPTH
  }
  if (0) // raw
  {
    double colRowRatio = (double)activeimg.cols / (double)activeimg.rows;
    int namedWindowRows = 600;
    int namedWindowCols = (double)namedWindowRows * colRowRatio;
    cv::namedWindow("activeimg_imshow_raw", cv::WINDOW_NORMAL);
    cv::resizeWindow("activeimg_imshow_raw", namedWindowCols, namedWindowRows);
    imshow("activeimg_imshow_raw", activeimg);
  }
  activeimg.convertTo(activeimg, CV_16U);
  normalize(activeimg, activeimg, 0, 65535, cv::NORM_MINMAX);
  showimg(activeimg, "activeimg", false, quanBot, quanTop);
  std::cout << "image loaded" << std::endl;
}
