
#include "SDOipcOpt.h"
#include "Optimization/Evolution.h"

double absoluteSubpixelRegistrationError(IPCsettings& set, const cv::Mat& src, double noisestddev, double maxShift, double accuracy)
{
  double returnVal = 0;
  cv::Mat srcCrop1, srcCrop2;
  cv::Mat crop1, crop2;
  std::vector<double> startFractionX = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
  std::vector<double> startFractionY = {0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7};
  // std::vector<double> startFractionX = { 0.5 };
  // std::vector<double> startFractionY = { 0.5 };
  int trials = (double)maxShift / accuracy + 1;
  for (size_t startposition = 0; startposition < startFractionX.size(); startposition++)
  {
    int startX = src.cols * startFractionX[startposition];
    int startY = src.rows * startFractionY[startposition];
    srcCrop1 = roicrop(src, startX, startY, 1.5 * set.getcols() + 2, 1.5 * set.getrows() + 2);
    crop1 = roicrop(srcCrop1, srcCrop1.cols / 2, srcCrop1.rows / 2, set.getcols(), set.getrows());
    for (int iterPic = 0; iterPic < trials; iterPic++)
    {
      double shiftX = (double)iterPic / (trials - 1) * maxShift;
      double shiftY = 0.;
      cv::Mat T = (cv::Mat_<float>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
      warpAffine(srcCrop1, srcCrop2, T, cv::Size(srcCrop1.cols, srcCrop1.rows));
      crop2 = roicrop(srcCrop2, srcCrop2.cols / 2, srcCrop2.rows / 2, set.getcols(), set.getrows());
      if (noisestddev)
      {
        crop1.convertTo(crop1, CV_32F);
        crop2.convertTo(crop2, CV_32F);
        cv::Mat noise1 = cv::Mat::zeros(crop1.rows, crop1.cols, CV_32F);
        cv::Mat noise2 = cv::Mat::zeros(crop1.rows, crop1.cols, CV_32F);
        randn(noise1, 0, noisestddev);
        randn(noise2, 0, noisestddev);
        crop1 += noise1;
        crop2 += noise2;
      }
      auto shift = phasecorrel(crop1, crop2, set);
      returnVal += abs(shift.x - shiftX);
      if constexpr (0) // DEBUG
      {
        showimg(crop1, "crop1");
        showimg(crop1, "crop2");
      }
    }
  }
  returnVal /= trials;
  returnVal /= startFractionX.size();
  return returnVal;
}

double IPCparOptFun(const std::vector<double>& args, const IPCsettings& settingsMaster, const cv::Mat& source, double noisestddev, double maxShift, double accuracy)
{
  IPCsettings settings = settingsMaster;
  settings.setBandpassParameters(args[0], args[1]);
  if (args.size() > 2)
    settings.L2size = std::max(3., std::abs(std::round(args[2])));
  if (args.size() > 3)
    settings.applyWindow = args[3] > 0 ? true : false;
  return absoluteSubpixelRegistrationError(settings, source, noisestddev, maxShift, accuracy);
}

void optimizeIPCParameters(const IPCsettings& settingsMaster, std::string pathInput, std::string pathOutput, double maxShift, double accuracy, size_t runs)
{
  std::ofstream listing(pathOutput, std::ios::out | std::ios::app);
  listing << "Running IPC parameter optimization (" << currentDateTime() << ")" << std::endl;
  listing << "filename,size,maxShift,stdevLmul,stdevHmul,L2,window,avgError,dateTime" << std::endl;
  cv::Mat pic = loadImage(pathInput);
  std::string windowname = "objective function source";
  showimg(pic, windowname);
  double noisestdev = 0;
  auto f = [&](const std::vector<double>& args) { return IPCparOptFun(args, settingsMaster, pic, noisestdev, maxShift, accuracy); };

  for (size_t iterOpt = 0; iterOpt < runs; iterOpt++)
  {
    Evolution Evo(4);
    Evo.mNP = 24;
    Evo.mMutStrat = Evolution::MutationStrategy::RAND1;
    Evo.mLB = std::vector<double>{0, 0, 3, -1};
    Evo.mUB = std::vector<double>{10, 200, 19, 1};
    auto Result = Evo.Optimize(f);
    // listing << pathInput << "," << settingsMaster.getcols() << "x" << settingsMaster.getrows() << "," << maxShift << "," << Result[0] << "," << Result[1] << "," << Result[2] << "," << Result[3] <<
    // "," << f(Result) << "," << currentDateTime() << std::endl;
  }
  cv::destroyWindow(windowname);
}

void optimizeIPCParametersForAllWavelengths(const IPCsettings& settingsMaster, double maxShift, double accuracy, size_t runs)
{
  std::ofstream listing("D:\\MainOutput\\IPC_parOpt.csv", std::ios::out | std::ios::trunc);
  if constexpr (1) // OPT 4par
  {
    listing << "Running IPC parameter optimization (" << currentDateTime() << "), image size " << settingsMaster.getcols() << std::endl;
    listing << "wavelength,stdevLmul,stdevHmul,L2,window,avgError,dateTime" << std::endl;
    for (size_t wavelength = 0; wavelength < WAVELENGTHS_STR.size(); wavelength++)
    {
      std::string path = "D:\\MainOutput\\png\\" + WAVELENGTHS_STR[wavelength] + "_proc.png";
      std::cout << "OPT .png load path: " << path << std::endl;
      cv::Mat pic = cv::imread(path, cv::IMREAD_ANYDEPTH);
      showimg(pic, "objfun current source");
      if constexpr (1) // optimize
      {
        auto f = [&](const std::vector<double>& args) { return IPCparOptFun(args, settingsMaster, pic, STDDEVS[wavelength], maxShift, accuracy); };
        for (size_t iterOpt = 0; iterOpt < runs; iterOpt++)
        {
          Evolution Evo(4);
          Evo.mNP = 25;
          Evo.mMutStrat = Evolution::MutationStrategy::RAND1;
          Evo.optimalFitness = 0;
          Evo.mLB = std::vector<double>{0, 0, 3, -1};
          Evo.mUB = std::vector<double>{20, 200, 19, 1};
          auto Result = Evo.Optimize(f);
          // listing << WAVELENGTHS_STR[wavelength] << "," << Result[0] << "," << Result[1] << "," << Result[2] << "," << Result[3] << "," << f(Result) << "," << currentDateTime() << std::endl;
        }
      }
    }
  }
}
