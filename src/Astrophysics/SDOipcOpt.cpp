
#include "SDOipcOpt.h"
#include "Optimization/Evolution.h"

f64 absoluteSubpixelRegistrationError(IPCsettings& set, const cv::Mat& src, f64 noisestddev, f64 maxShift, f64 accuracy)
{
  f64 returnVal = 0;
  cv::Mat srcCrop1, srcCrop2;
  cv::Mat crop1, crop2;
  std::vector<f64> startFractionX = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
  std::vector<f64> startFractionY = {0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7};
  // std::vector<f64> startFractionX = { 0.5 };
  // std::vector<f64> startFractionY = { 0.5 };
  i32 trials = (f64)maxShift / accuracy + 1;
  for (usize startposition = 0; startposition < startFractionX.size(); startposition++)
  {
    i32 startX = src.cols * startFractionX[startposition];
    i32 startY = src.rows * startFractionY[startposition];
    srcCrop1 = roicrop(src, startX, startY, 1.5 * set.getcols() + 2, 1.5 * set.getrows() + 2);
    crop1 = roicrop(srcCrop1, srcCrop1.cols / 2, srcCrop1.rows / 2, set.getcols(), set.getrows());
    for (i32 iterPic = 0; iterPic < trials; iterPic++)
    {
      f64 shiftX = (f64)iterPic / (trials - 1) * maxShift;
      f64 shiftY = 0.;
      cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
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

f64 IPCparOptFun(const std::vector<f64>& args, const IPCsettings& settingsMaster, const cv::Mat& source, f64 noisestddev, f64 maxShift, f64 accuracy)
{
  IPCsettings settings = settingsMaster;
  settings.setBandpassParameters(args[0], args[1]);
  if (args.size() > 2)
    settings.L2size = std::max(3., std::abs(std::round(args[2])));
  if (args.size() > 3)
    settings.applyWindow = args[3] > 0 ? true : false;
  return absoluteSubpixelRegistrationError(settings, source, noisestddev, maxShift, accuracy);
}

void optimizeIPCParameters(const IPCsettings& settingsMaster, std::string pathInput, std::string pathOutput, f64 maxShift, f64 accuracy, usize runs)
{
  std::ofstream listing(pathOutput, std::ios::out | std::ios::app);
  listing << "Running IPC parameter optimization (" << currentDateTime() << ")" << std::endl;
  listing << "filename,size,maxShift,stdevLmul,stdevHmul,L2,window,avgError,dateTime" << std::endl;
  cv::Mat pic = loadImage(pathInput);
  std::string windowname = "objective function source";
  showimg(pic, windowname);
  f64 noisestdev = 0;
  auto f = [&](const std::vector<f64>& args) { return IPCparOptFun(args, settingsMaster, pic, noisestdev, maxShift, accuracy); };

  for (usize iterOpt = 0; iterOpt < runs; iterOpt++)
  {
    Evolution Evo(4);
    Evo.mNP = 24;
    Evo.mMutStrat = Evolution::MutationStrategy::RAND1;
    Evo.mLB = std::vector<f64>{0, 0, 3, -1};
    Evo.mUB = std::vector<f64>{10, 200, 19, 1};
    auto Result = Evo.Optimize(f);
    // listing << pathInput << "," << settingsMaster.getcols() << "x" << settingsMaster.getrows() << "," << maxShift << "," << Result[0] << "," << Result[1] << "," << Result[2] << "," << Result[3] <<
    // "," << f(Result) << "," << currentDateTime() << std::endl;
  }
  cv::destroyWindow(windowname);
}

void optimizeIPCParametersForAllWavelengths(const IPCsettings& settingsMaster, f64 maxShift, f64 accuracy, usize runs)
{
  std::ofstream listing("D:\\MainOutput\\IPC_parOpt.csv", std::ios::out | std::ios::trunc);
  if constexpr (1) // OPT 4par
  {
    listing << "Running IPC parameter optimization (" << currentDateTime() << "), image size " << settingsMaster.getcols() << std::endl;
    listing << "wavelength,stdevLmul,stdevHmul,L2,window,avgError,dateTime" << std::endl;
    for (usize wavelength = 0; wavelength < WAVELENGTHS_STR.size(); wavelength++)
    {
      std::string path = "D:\\MainOutput\\png\\" + WAVELENGTHS_STR[wavelength] + "_proc.png";
      std::cout << "OPT .png load path: " << path << std::endl;
      cv::Mat pic = cv::imread(path, cv::IMREAD_ANYDEPTH);
      showimg(pic, "objfun current source");
      if constexpr (1) // optimize
      {
        auto f = [&](const std::vector<f64>& args) { return IPCparOptFun(args, settingsMaster, pic, STDDEVS[wavelength], maxShift, accuracy); };
        for (usize iterOpt = 0; iterOpt < runs; iterOpt++)
        {
          Evolution Evo(4);
          Evo.mNP = 25;
          Evo.mMutStrat = Evolution::MutationStrategy::RAND1;
          Evo.mOptimalFitness = 0;
          Evo.mLB = std::vector<f64>{0, 0, 3, -1};
          Evo.mUB = std::vector<f64>{20, 200, 19, 1};
          auto Result = Evo.Optimize(f);
          // listing << WAVELENGTHS_STR[wavelength] << "," << Result[0] << "," << Result[1] << "," << Result[2] << "," << Result[3] << "," << f(Result) << "," << currentDateTime() << std::endl;
        }
      }
    }
  }
}
