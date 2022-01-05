#include "IterativePhaseCorrelation.h"

cv::Mat IterativePhaseCorrelation::Align(cv::Mat&& image1, cv::Mat&& image2) const
try
{
  cv::Mat img1W = image1.clone();
  cv::Mat img2W = image2.clone();
  ApplyWindow<false>(img1W);
  ApplyWindow<false>(img2W);
  cv::Mat img1FT = Fourier::fft(img1W);
  cv::Mat img2FT = Fourier::fft(img2W);
  Fourier::fftshift(img1FT);
  Fourier::fftshift(img2FT);
  cv::Mat img1FTm = cv::Mat::zeros(img1FT.size(), CV_32F);
  cv::Mat img2FTm = cv::Mat::zeros(img2FT.size(), CV_32F);
  for (i32 row = 0; row < img1FT.rows; ++row)
  {
    auto img1FTp = img1FT.ptr<cv::Vec2f>(row);
    auto img2FTp = img2FT.ptr<cv::Vec2f>(row);
    auto img1FTmp = img1FTm.ptr<f32>(row);
    auto img2FTmp = img2FTm.ptr<f32>(row);
    for (i32 col = 0; col < img1FT.cols; ++col)
    {
      const f32& re1 = img1FTp[col][0];
      const f32& im1 = img1FTp[col][1];
      const f32& re2 = img2FTp[col][0];
      const f32& im2 = img2FTp[col][1];
      img1FTmp[col] = log(sqrt(re1 * re1 + im1 * im1));
      img2FTmp[col] = log(sqrt(re2 * re2 + im2 * im2));
    }
  }
  cv::Point2f center((f32)image1.cols / 2, (f32)image1.rows / 2);
  f64 maxRadius = std::min(center.y, center.x);
  warpPolar(img1FTm, img1FTm, img1FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar
  warpPolar(img2FTm, img2FTm, img2FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar

  // rotation
  auto shiftR = Calculate(img1FTm, img2FTm);
  f64 rotation = -shiftR.y / image1.rows * 360;
  f64 scale = exp(shiftR.x * log(maxRadius) / image1.cols);
  Rotate(image2, -rotation, scale);

  // translation
  auto shiftT = Calculate(image1, image2);
  Shift(image2, -shiftT);

  if constexpr (0)
  {
    LOG_INFO("Evaluated rotation: {} deg", rotation);
    LOG_INFO("Evaluated scale: {}", 1.f / scale);
    LOG_INFO("Evaluated shift: {} px", shiftT);
    showimg(ColorComposition(image1, image2), "color composition result", 0, 0, 1, 1000);
  }
  return image2;
}
catch (const std::exception& e)
{
  LOG_ERROR("IPC Align error: {}", e.what());
  return cv::Mat();
}

std::tuple<cv::Mat, cv::Mat> IterativePhaseCorrelation::CalculateFlow(cv::Mat&& image1, cv::Mat&& image2, f32 resolution) const
try
{
  if (image1.size() != image2.size())
    throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

  if (mRows > image1.rows or mCols > image1.cols)
    throw std::runtime_error(fmt::format("Images are too small ({} < {})", image1.size(), cv::Size(mCols, mRows)));

  cv::Mat flowX = cv::Mat::zeros(cv::Size(resolution * image1.cols, resolution * image1.rows), CV_32F);
  cv::Mat flowY = cv::Mat::zeros(cv::Size(resolution * image2.cols, resolution * image2.rows), CV_32F);
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 r = 0; r < flowX.rows; ++r)
  {
    if (++progress % (flowX.rows / 20) == 0)
      LOG_DEBUG("Calculating IPC flow profile ({:.0f}%)", static_cast<f32>(progress) / flowX.rows * 100);

    for (i32 c = 0; c < flowX.cols; ++c)
    {
      const cv::Point2i center(c / resolution, r / resolution);

      if (IsOutOfBounds(center, image1, {mCols, mRows}))
        continue;

      const auto shift = Calculate(roicrop(image1, center.x, center.y, mCols, mRows), roicrop(image2, center.x, center.y, mCols, mRows));
      flowX.at<f32>(r, c) = shift.x;
      flowY.at<f32>(r, c) = shift.y;
    }
  }

  return {flowX, flowY};
}
catch (const std::exception& e)
{
  LOG_ERROR("IPC CalculateFlow error: {}", e.what());
  return {};
}

cv::Mat IterativePhaseCorrelation::ColorComposition(const cv::Mat& img1, const cv::Mat& img2)
{
  const cv::Vec3f img1clr = {1, 0.5, 0};
  const cv::Vec3f img2clr = {0, 0.5, 1};

  const f32 gamma1 = 1.0;
  const f32 gamma2 = 1.0;

  cv::Mat img1c = cv::Mat::zeros(img1.size(), CV_32FC3);
  cv::Mat img2c = cv::Mat::zeros(img2.size(), CV_32FC3);

  for (i32 row = 0; row < img1.rows; ++row)
  {
    auto img1p = img1.ptr<f32>(row);
    auto img2p = img2.ptr<f32>(row);
    auto img1cp = img1c.ptr<cv::Vec3f>(row);
    auto img2cp = img2c.ptr<cv::Vec3f>(row);

    for (i32 col = 0; col < img1.cols; ++col)
    {
      img1cp[col][0] = pow(img1clr[2] * img1p[col], 1. / gamma1);
      img1cp[col][1] = pow(img1clr[1] * img1p[col], 1. / gamma1);
      img1cp[col][2] = pow(img1clr[0] * img1p[col], 1. / gamma1);

      img2cp[col][0] = pow(img2clr[2] * img2p[col], 1. / gamma2);
      img2cp[col][1] = pow(img2clr[1] * img2p[col], 1. / gamma2);
      img2cp[col][2] = pow(img2clr[0] * img2p[col], 1. / gamma2);
    }
  }

  if (gamma1 != 1 or gamma2 != 1)
  {
    normalize(img1c, img1c, 0, 1, cv::NORM_MINMAX);
    normalize(img2c, img2c, 0, 1, cv::NORM_MINMAX);
  }

  return (img1c + img2c) / 2;
}

std::vector<cv::Mat> IterativePhaseCorrelation::LoadImages(const std::string& imagesDirectory, bool mute)
{
  if (!mute)
    LOG_INFO("Loading images from '{}'...", imagesDirectory);

  if (!std::filesystem::is_directory(imagesDirectory))
    throw std::runtime_error(fmt::format("Directory '{}' is not a valid directory", imagesDirectory));

  std::vector<cv::Mat> images;
  for (const auto& entry : std::filesystem::directory_iterator(imagesDirectory))
  {
    const std::string path = entry.path().string();

    if (!IsImage(path))
    {
      if (!mute)
        LOG_DEBUG("Directory contains a non-image file {}", path);
      continue;
    }

    // crop the input image - good for solar images, omits the black borders
    static constexpr f32 cropFocusRatio = 0.5;
    auto image = loadImage(path);
    image = roicropmid(image, cropFocusRatio * image.cols, cropFocusRatio * image.rows);
    images.push_back(image);
    if (!mute)
      LOG_DEBUG("Loaded image {}", path);
  }
  return images;
}

std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>> IterativePhaseCorrelation::CreateImagePairs(const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStdev) const
{
  for (const auto& image : images)
  {
    if (image.rows < mRows + maxShift or image.cols < mCols + maxShift)
      throw std::runtime_error(fmt::format("Could not optimize IPC parameters - input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])", image.rows, image.cols,
          mRows + maxShift, mCols + maxShift));
  }

  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>> imagePairs;
  imagePairs.reserve(images.size() * itersPerImage);

  for (const auto& image : images)
  {
    for (i32 i = 0; i < itersPerImage; ++i)
    {
      // random shift from a random point
      cv::Point2f shift(rand11() * maxShift, rand11() * maxShift);
      cv::Point2i point(clamp(rand01() * image.cols, mCols, image.cols - mCols), clamp(rand01() * image.rows, mRows, image.rows - mRows));
      cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      warpAffine(image, imageShifted, T, image.size());
      cv::Mat image1 = roicrop(image, point.x, point.y, mCols, mRows);
      cv::Mat image2 = roicrop(imageShifted, point.x, point.y, mCols, mRows);

      ConvertToUnitFloat<false>(image1);
      ConvertToUnitFloat<false>(image2);

      AddNoise(image1, noiseStdev);
      AddNoise(image2, noiseStdev);

      imagePairs.push_back({image1, image2, shift});

      if constexpr (0)
      {
        cv::Mat hcct;
        hconcat(image1, image2, hcct);
        showimg(hcct, fmt::format("IPC optimization pair {}", i));
      }
    }
  }
  return imagePairs;
}

void IterativePhaseCorrelation::AddNoise(cv::Mat& image, f64 noiseStdev)
{
  if (noiseStdev <= 0)
    return;

  cv::Mat noise = cv::Mat::zeros(image.rows, image.cols, CV_32F);
  randn(noise, 0, noiseStdev);
  image += noise;
}

const std::function<f64(const std::vector<f64>&)> IterativePhaseCorrelation::CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
{
  return [&](const std::vector<f64>& params)
  {
    IterativePhaseCorrelation ipc(mRows, mCols);
    ipc.SetBandpassType(static_cast<BandpassType>((i32)params[BandpassTypeParameter]));
    ipc.SetBandpassParameters(params[BandpassLParameter], params[BandpassHParameter]);
    ipc.SetInterpolationType(static_cast<InterpolationType>((i32)params[InterpolationTypeParameter]));
    ipc.SetWindowType(static_cast<WindowType>((i32)params[WindowTypeParameter]));
    ipc.SetUpsampleCoeff(params[UpsampleCoeffParameter]);
    ipc.SetL1ratio(params[L1ratioParameter]);

    if (std::floor(ipc.GetL2size() * ipc.GetUpsampleCoeff() * ipc.GetL1ratio()) < 3)
      return std::numeric_limits<f64>::max();

    f64 avgerror = 0;
    for (const auto& [image1, image2, shift] : imagePairs)
    {
      const auto error = ipc.Calculate(image1, image2) - shift;
      avgerror += sqrt(error.x * error.x + error.y * error.y);
    }
    return imagePairs.size() > 0 ? avgerror / imagePairs.size() : 0.0f;
  };
}

std::vector<f64> IterativePhaseCorrelation::CalculateOptimalParameters(
    const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize, bool mute)
{
  Evolution evo(ParameterCount);
  evo.mNP = populationSize;
  evo.mMutStrat = Evolution::RAND1;
  evo.SetParameterNames({"BPT", "BPL", "BPH", "ITPT", "WINT", "UC", "L1R"});
  evo.mLB = {0, -.5, 0.0, 0, 0, 11, 0.1};
  evo.mUB = {2, 1.0, 1.5, 3, 2, 51, 0.8};
  evo.SetPlotOutput(!mute);
  evo.SetConsoleOutput(!mute);
  return evo.Optimize(obj, valid).optimum;
}

void IterativePhaseCorrelation::ApplyOptimalParameters(const std::vector<f64>& optimalParameters, bool mute)
{
  if (optimalParameters.size() != ParameterCount)
    throw std::runtime_error("Cannot apply optimal parameters - wrong parameter count");

  SetBandpassType(static_cast<BandpassType>((i32)optimalParameters[BandpassTypeParameter]));
  SetBandpassParameters(optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]);
  SetInterpolationType(static_cast<InterpolationType>((i32)optimalParameters[InterpolationTypeParameter]));
  SetWindowType(static_cast<WindowType>((i32)optimalParameters[WindowTypeParameter]));
  SetUpsampleCoeff(optimalParameters[UpsampleCoeffParameter]);
  SetL1ratio(optimalParameters[L1ratioParameter]);

  if (!mute)
  {
    LOG_INFO("Final IPC BandpassType: {}",
        BandpassType2String(static_cast<BandpassType>((i32)optimalParameters[BandpassTypeParameter]), optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]));
    LOG_INFO("Final IPC BandpassL: {:.2f}", optimalParameters[BandpassLParameter]);
    LOG_INFO("Final IPC BandpassH: {:.2f}", optimalParameters[BandpassHParameter]);
    LOG_INFO("Final IPC InterpolationType: {}", InterpolationType2String(static_cast<InterpolationType>((i32)optimalParameters[InterpolationTypeParameter])));
    LOG_INFO("Final IPC WindowType: {}", WindowType2String(static_cast<WindowType>((i32)optimalParameters[WindowTypeParameter])));
    LOG_INFO("Final IPC UpsampleCoeff: {}", static_cast<i32>(optimalParameters[UpsampleCoeffParameter]));
    LOG_INFO("Final IPC L1ratio: {:.2f}", optimalParameters[L1ratioParameter]);
  }
}

std::string IterativePhaseCorrelation::BandpassType2String(BandpassType type, f64 bandpassL, f64 bandpassH) const
{
  switch (type)
  {
  case BandpassType::Rectangular:
    if (mBandpassL <= 0 and mBandpassH < 1)
      return "Rectangular low pass";
    else if (mBandpassL > 0 and mBandpassH >= 1)
      return "Rectangular high pass";
    else if (mBandpassL > 0 and mBandpassH < 1)
      return "Rectangular band pass";
    else if (mBandpassL <= 0 and mBandpassH >= 1)
      return "Rectangular all pass";
    else
      throw std::runtime_error("Unknown bandpass type");
  case BandpassType::Gaussian:
    if (mBandpassL <= 0 and mBandpassH < 1)
      return "Gaussian low pass";
    else if (mBandpassL > 0 and mBandpassH >= 1)
      return "Gaussian high pass";
    else if (mBandpassL > 0 and mBandpassH < 1)
      return "Gausian band pass";
    else if (mBandpassL <= 0 and mBandpassH >= 1)
      return "Gaussian all pass";
    else
      throw std::runtime_error("Unknown bandpass type");
  default:
    throw std::runtime_error("Unknown bandpass type");
  }
}

std::string IterativePhaseCorrelation::WindowType2String(WindowType type)
{
  switch (type)
  {
  case WindowType::Rectangular:
    return "Rectangular";
  case WindowType::Hann:
    return "Hann";
  default:
    throw std::runtime_error("Unknown window type");
  }
}

std::string IterativePhaseCorrelation::InterpolationType2String(InterpolationType type)
{
  switch (type)
  {
  case InterpolationType::NearestNeighbor:
    return "NearestNeighbor";
  case InterpolationType::Linear:
    return "Linear";
  case InterpolationType::Cubic:
    return "Cubic";
  default:
    throw std::runtime_error("Unknown interpolation type");
  }
}

void IterativePhaseCorrelation::ShowOptimizationPlots(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shiftsPixel, const std::vector<cv::Point2f>& shiftsNonit,
    const std::vector<cv::Point2f>& shiftsBefore, const std::vector<cv::Point2f>& shiftsAfter)
{
  std::vector<f64> shiftsXReference, shiftsXReferenceError;
  std::vector<f64> shiftsXPixel, shiftsXPixelError;
  std::vector<f64> shiftsXNonit, shiftsXNonitError;
  std::vector<f64> shiftsXBefore, shiftsXBeforeError;
  std::vector<f64> shiftsXAfter, shiftsXAfterError;

  for (usize i = 0; i < shiftsReference.size(); ++i)
  {
    const auto& referenceShift = shiftsReference[i];
    const auto& shiftPixel = shiftsPixel[i];
    const auto& shiftNonit = shiftsNonit[i];
    const auto& shiftBefore = shiftsBefore[i];

    shiftsXReference.push_back(referenceShift.x);
    shiftsXReferenceError.push_back(0.0);

    shiftsXPixel.push_back(shiftPixel.x);
    shiftsXNonit.push_back(shiftNonit.x);
    shiftsXBefore.push_back(shiftBefore.x);

    shiftsXPixelError.push_back(shiftPixel.x - referenceShift.x);
    shiftsXNonitError.push_back(shiftNonit.x - referenceShift.x);
    shiftsXBeforeError.push_back(shiftBefore.x - referenceShift.x);

    if (i < shiftsAfter.size())
    {
      const auto& shiftAfter = shiftsAfter[i];

      shiftsXAfter.push_back(shiftAfter.x);
      shiftsXAfterError.push_back(shiftAfter.x - referenceShift.x);
    }
  }

  Plot1D::Set("IPCshift");
  Plot1D::SetXlabel("reference shift");
  Plot1D::SetYlabel("calculated shift");
  Plot1D::SetYnames({"reference", "pixel", "subpixel", "ipc", "ipc opt"});
  Plot1D::SetPens(
      {QPen(Plot::blue, Plot::pt / 2, Qt::DashLine), QPen(Plot::black, Plot::pt / 2, Qt::DashLine), QPen(Plot::orange, Plot::pt), QPen(Plot::magenta, Plot::pt), QPen(Plot::green, Plot::pt)});
  Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
  Plot1D::Plot("IPCshift", shiftsXReference, {shiftsXReference, shiftsXPixel, shiftsXNonit, shiftsXBefore, shiftsXAfter});

  Plot1D::Set("IPCshifterror");
  Plot1D::SetXlabel("reference shift");
  Plot1D::SetYlabel("pixel error");
  Plot1D::SetYnames({"reference", "pixel", "subpixel", "ipc", "ipc opt"});
  Plot1D::SetPens(
      {QPen(Plot::blue, Plot::pt / 2, Qt::DashLine), QPen(Plot::black, Plot::pt / 2, Qt::DashLine), QPen(Plot::orange, Plot::pt), QPen(Plot::magenta, Plot::pt), QPen(Plot::green, Plot::pt)});
  Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
  Plot1D::Plot("IPCshifterror", shiftsXReference, {shiftsXReferenceError, shiftsXPixelError, shiftsXNonitError, shiftsXBeforeError, shiftsXAfterError});
}

std::vector<cv::Point2f> IterativePhaseCorrelation::GetShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
{
  std::vector<cv::Point2f> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(Calculate(image1, image2));

  return out;
}

std::vector<cv::Point2f> IterativePhaseCorrelation::GetNonIterativeShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
{
  std::vector<cv::Point2f> out;
  out.reserve(imagePairs.size());

  mAccuracyType = AccuracyType::Subpixel;
  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(Calculate(image1, image2));
  mAccuracyType = AccuracyType::SubpixelIterative;

  return out;
}

std::vector<cv::Point2f> IterativePhaseCorrelation::GetPixelShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
{
  std::vector<cv::Point2f> out;
  out.reserve(imagePairs.size());

  mAccuracyType = AccuracyType::Pixel;
  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(Calculate(image1, image2));
  mAccuracyType = AccuracyType::SubpixelIterative;

  return out;
}

std::vector<cv::Point2f> IterativePhaseCorrelation::GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs)
{
  std::vector<cv::Point2f> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(referenceShift);

  return out;
}

f64 IterativePhaseCorrelation::GetAverageAccuracy(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shifts)
{
  if (shiftsReference.size() != shifts.size())
    throw std::runtime_error("Reference shift vector has different size than calculated shift vector");

  f64 avgerror = 0;
  for (usize i = 0; i < shifts.size(); ++i)
  {
    const auto error = shifts[i] - shiftsReference[i];
    avgerror += sqrt(error.x * error.x + error.y * error.y);
  }
  return avgerror / shifts.size();
}

void IterativePhaseCorrelation::ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs)
{
  const auto& [img1, img2, shift] = imagePairs[static_cast<usize>(rand01() * imagePairs.size())];
  cv::Mat concat;
  hconcat(img1, img2, concat);
  Plot2D::Set("Random image pair");
  Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
  Plot2D::Plot("Random image pair", concat);
}

f64 IterativePhaseCorrelation::GetFractionalPart(f64 x)
{
  return abs(x - std::floor(x));
}

void IterativePhaseCorrelation::Optimize(
    const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, f64 validationRatio, i32 populationSize, bool mute)
try
{
  if (!mute)
    LOG_FUNCTION("IPC optimization");

  if (itersPerImage < 1)
    throw std::runtime_error(fmt::format("Invalid iters per image ({})", itersPerImage));
  if (maxShift <= 0)
    throw std::runtime_error(fmt::format("Invalid max shift ({})", maxShift));
  if (noiseStdev < 0)
    throw std::runtime_error(fmt::format("Invalid noise stdev ({})", noiseStdev));

  const auto trainingImages = LoadImages(trainingImagesDirectory, mute);
  const auto validationImages = LoadImages(validationImagesDirectory, mute);

  if (trainingImages.empty())
    throw std::runtime_error("Empty training images vector");

  const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
  const auto validationImagePairs = CreateImagePairs(validationImages, maxShift, validationRatio * itersPerImage, noiseStdev);

  std::vector<cv::Point2f> referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore;
  f64 objBefore;

  // before
  if (!mute)
  {
    LOG_INFO("Running Iterative Phase Correlation parameter optimization on a set of {}/{} training/validation images with {}/{} image pairs - each generation, {} {}x{} IPCshifts will be calculated",
        trainingImages.size(), validationImages.size(), trainingImagePairs.size(), validationImagePairs.size(), populationSize * trainingImagePairs.size() + validationImagePairs.size(), mCols, mRows);
    ShowRandomImagePair(trainingImagePairs);
    referenceShifts = GetReferenceShifts(trainingImagePairs);
    shiftsPixel = GetPixelShifts(trainingImagePairs);
    shiftsNonit = GetNonIterativeShifts(trainingImagePairs);
    shiftsBefore = GetShifts(trainingImagePairs);
    objBefore = GetAverageAccuracy(referenceShifts, shiftsBefore);
    ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, {});
  }

  // opt
  const auto obj = CreateObjectiveFunction(trainingImagePairs);
  const auto valid = CreateObjectiveFunction(validationImagePairs);
  const auto optimalParameters = CalculateOptimalParameters(obj, valid, populationSize, mute);
  if (optimalParameters.empty())
    throw std::runtime_error("Optimization failed");
  ApplyOptimalParameters(optimalParameters, mute);

  // after
  if (!mute)
  {
    const auto shiftsAfter = GetShifts(trainingImagePairs);
    const auto objAfter = GetAverageAccuracy(referenceShifts, shiftsAfter);
    ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, shiftsAfter);
    LOG_INFO("Average pixel accuracy improvement: {:.3f} -> {:.3f} ({}%)", objBefore, objAfter, static_cast<i32>((objBefore - objAfter) / objBefore * 100));
    LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
  }
}
catch (const std::exception& e)
{
  LOG_ERROR("An error occured during Iterative Phase Correlation parameter optimization: {}", e.what());
}

void IterativePhaseCorrelation::PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const
{
  LOG_FUNCTION("PlotObjectiveFunctionLandscape");
  const auto trainingImages = LoadImages(trainingImagesDirectory);
  const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
  const auto obj = CreateObjectiveFunction(trainingImagePairs);
  const i32 rows = iters;
  const i32 cols = iters;
  cv::Mat landscape(rows, cols, CV_32F);
  const f64 xmin = -0.25;
  const f64 xmax = 0.75;
  const f64 ymin = 0.25;
  const f64 ymax = 1.25;
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 r = 0; r < rows; ++r)
  {
    for (i32 c = 0; c < cols; ++c)
    {
      LOG_INFO("Calculating objective function landscape ({:.1f}%)", (f32)progress / (rows * cols - 1) * 100);
      std::vector<f64> parameters(ParameterCount);

      // default
      parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
      parameters[BandpassLParameter] = mBandpassL;
      parameters[BandpassHParameter] = mBandpassH;
      parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
      parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
      parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
      parameters[L1ratioParameter] = mL1ratio;

      // modified
      parameters[BandpassLParameter] = xmin + (f64)c / (cols - 1) * (xmax - xmin);
      parameters[BandpassHParameter] = ymin + (f64)r / (rows - 1) * (ymax - ymin);

      landscape.at<f32>(r, c) = std::log(obj(parameters));
      progress++;
    }
  }

  Plot2D::Set("IPCdebug2D");
  Plot2D::SetXmin(xmin);
  Plot2D::SetXmax(xmax);
  Plot2D::SetYmin(ymin);
  Plot2D::SetYmax(ymax);
  Plot2D::Plot("IPCdebug2D", landscape);
}

void IterativePhaseCorrelation::PlotImageSizeAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters)
{
  const auto trainingImages = LoadImages(trainingImagesDirectory);
  std::vector<f64> imageSizes(iters);
  std::vector<f64> accuracy(iters);
  const f64 xmin = 16;
  const f64 xmax = mRows;
  std::atomic<i32> progress = 0;

  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating image size accuracy dependence ({:.1f}%)", (f32)progress / (iters - 1) * 100);
    i32 imageSize = xmin + (f64)i / (iters - 1) * (xmax - xmin);
    imageSize = imageSize % 2 ? imageSize + 1 : imageSize;
    SetSize(imageSize, imageSize);
    const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
    const auto obj = CreateObjectiveFunction(trainingImagePairs);
    std::vector<f64> parameters(ParameterCount);

    // default
    parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
    parameters[BandpassLParameter] = mBandpassL;
    parameters[BandpassHParameter] = mBandpassH;
    parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
    parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
    parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
    parameters[L1ratioParameter] = mL1ratio;

    imageSizes[i] = imageSize;
    accuracy[i] = obj(parameters);
    progress++;
  }

  Plot1D::Set("ImageSizeAccuracyDependence");
  Plot1D::SetXlabel("Image size");
  Plot1D::SetYlabel("Average pixel error");
  // Plot1D::SetYLogarithmic(true);
  Plot1D::Plot("ImageSizeAccuracyDependence", imageSizes, accuracy);
}

void IterativePhaseCorrelation::PlotUpsampleCoefficientAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const
{
  const auto trainingImages = LoadImages(trainingImagesDirectory);
  const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
  const auto obj = CreateObjectiveFunction(trainingImagePairs);

  std::vector<f64> upsampleCoeff(iters);
  std::vector<f64> accuracy(iters);
  const f64 xmin = 1;
  const f64 xmax = 35;
  std::atomic<i32> progress = 0;

  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating upsample coeffecient accuracy dependence ({:.1f}%)", (f32)progress / (iters - 1) * 100);
    std::vector<f64> parameters(ParameterCount);

    // default
    parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
    parameters[BandpassLParameter] = mBandpassL;
    parameters[BandpassHParameter] = mBandpassH;
    parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
    parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
    parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
    parameters[L1ratioParameter] = mL1ratio;

    // modified
    parameters[UpsampleCoeffParameter] = xmin + (f64)i / (iters - 1) * (xmax - xmin);

    upsampleCoeff[i] = parameters[UpsampleCoeffParameter];
    accuracy[i] = obj(parameters);
    progress++;
  }

  Plot1D::Set("UpsampleCoefficientAccuracyDependence");
  Plot1D::SetXlabel("Upsample coefficient");
  Plot1D::SetYlabel("Average pixel error");
  Plot1D::SetYLogarithmic(true);
  Plot1D::Plot("UpsampleCoefficientAccuracyDependence", upsampleCoeff, accuracy);
}

void IterativePhaseCorrelation::PlotNoiseAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const
{
  if (noiseStdev <= 0.0f)
  {
    LOG_ERROR("Please set some non-zero positive max noise stdev");
    return;
  }

  const auto trainingImages = LoadImages(trainingImagesDirectory);
  std::vector<f64> noiseStdevs(iters);
  std::vector<f64> accuracy(iters);
  const f64 xmin = 0;
  const f64 xmax = noiseStdev;
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating noise stdev accuracy dependence ({:.1f}%)", (f32)progress / (iters - 1) * 100);
    f32 noise = xmin + (f64)i / (iters - 1) * (xmax - xmin);
    const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noise);
    const auto obj = CreateObjectiveFunction(trainingImagePairs);
    std::vector<f64> parameters(ParameterCount);

    // default
    parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
    parameters[BandpassLParameter] = mBandpassL;
    parameters[BandpassHParameter] = mBandpassH;
    parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
    parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
    parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
    parameters[L1ratioParameter] = mL1ratio;

    noiseStdevs[i] = noise;
    accuracy[i] = obj(parameters);
    progress++;
  }

  Plot1D::Set("NoiseAccuracyDependence");
  Plot1D::SetXlabel("Noise stdev");
  Plot1D::SetYlabel("Average pixel error");
  Plot1D::Plot("NoiseAccuracyDependence", noiseStdevs, accuracy);
}

void IterativePhaseCorrelation::PlotNoiseOptimalBPHDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const
{
  if (noiseStdev <= 0.0f)
  {
    LOG_ERROR("Please set some non-zero positive max noise stdev");
    return;
  }

  std::vector<f64> noiseStdevs(iters);
  std::vector<f64> optimalBPHs(iters);
  const f64 xmin = 0;
  const f64 xmax = noiseStdev;
  i32 progress = 0;

  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating noise stdev optimal BPH dependence ({:.1f}%)", (f32)progress / (iters - 1) * 100);
    f32 noise = xmin + (f64)i / (iters - 1) * (xmax - xmin);

    IterativePhaseCorrelation ipc = *this; // copy this
    ipc.Optimize(trainingImagesDirectory, trainingImagesDirectory, 2.0f, noise, itersPerImage, 0.0f, ParameterCount * 2, true);

    noiseStdevs[i] = noise;
    optimalBPHs[i] = ipc.GetBandpassH();
    progress++;
  }

  Plot1D::Set("NoiseOptimalBPHDependence");
  Plot1D::SetXlabel("Noise stdev");
  Plot1D::SetYlabel("Optimal BPH");
  Plot1D::Plot("NoiseOptimalBPHDependence", noiseStdevs, optimalBPHs);
}

void IterativePhaseCorrelation::ShowDebugStuff() const
try
{
  constexpr bool DebugMode = true;
  constexpr bool debugShift = true;
  constexpr bool debugGradualShift = false;
  constexpr bool debugWindow = false;
  constexpr bool debugBandpass = false;
  constexpr bool debugBandpassRinging = false;

  if constexpr (debugShift)
  {
    std::string path1 = "../resources/AIA/171A.png";
    std::string path2 = "../resources/AIA/171A.png";
    bool artificialShift = path1 == path2;
    cv::Point2f rawshift = artificialShift ? cv::Point2f(rand11() * 0.25 * mCols, rand11() * 0.25 * mRows) : cv::Point2f(0, 0);
    cv::Mat image1 = loadImage(path1);
    cv::Mat image2 = artificialShift ? image1.clone() : loadImage(path2);
    image1 = roicrop(image1, image1.cols / 2, image1.rows / 2, mCols, mRows);
    image2 = roicrop(image2, image2.cols / 2 - rawshift.x, image2.rows / 2 - rawshift.y, mCols, mRows);
    bool addNoise = false;

    if (addNoise)
    {
      f64 noiseStdev = 0.03;
      cv::Mat noise1 = cv::Mat::zeros(image1.rows, image1.cols, CV_32F);
      cv::Mat noise2 = cv::Mat::zeros(image2.rows, image2.cols, CV_32F);
      randn(noise1, 0, noiseStdev);
      randn(noise2, 0, noiseStdev);
      image1 += noise1;
      image2 += noise2;
    }

    auto ipcshift = Calculate<DebugMode>(image1, image2);

    if (artificialShift)
      LOG_INFO("Artificial shift = {} / Estimate shift = {} / Error = {}", rawshift, ipcshift, ipcshift - rawshift);
    else
      LOG_INFO("Estimate shift = {}", ipcshift);
  }

  if constexpr (debugGradualShift)
  {
    SetDebugDirectory("Debug");
    const cv::Mat image1 = loadImage("../resources/AIA/171A.png");
    const cv::Mat crop1 = roicrop(image1, image1.cols / 2, image1.rows / 2, mCols, mRows);
    cv::Mat image2 = image1.clone();
    cv::Mat crop2;
    const i32 iters = 51;
    constexpr bool addNoise = true;
    f64 noiseStdev = 0.03;
    cv::Mat noise1, noise2;

    if constexpr (addNoise)
    {
      noise1 = cv::Mat::zeros(crop1.rows, crop1.cols, CV_32F);
      noise2 = cv::Mat::zeros(crop1.rows, crop1.cols, CV_32F);
      randn(noise1, 0, noiseStdev);
      randn(noise2, 0, noiseStdev);
      crop1 += noise1;
    }

    for (i32 i = 0; i < iters; i++)
    {
      SetDebugName(fmt::format("GradualShift{}", i));
      const cv::Point2f rawshift(static_cast<f32>(i) / (iters - 1), 0);
      const cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., rawshift.x, 0., 1., rawshift.y);
      warpAffine(image1, image2, T, image2.size());
      crop2 = roicrop(image2, image2.cols / 2, image2.rows / 2, mCols, mRows);
      if (addNoise)
        crop2 += noise2;
      const auto ipcshift = Calculate<DebugMode>(crop1, crop2);
      LOG_INFO("Artificial shift = {} / Estimate shift = {} / Error = {}", rawshift, ipcshift, ipcshift - rawshift);
    }
  }

  if constexpr (debugWindow)
  {
    cv::Mat img = roicrop(loadImage("../resources/test.png"), 2048, 2048, mCols, mRows);
    cv::Mat w, imgw;
    createHanningWindow(w, img.size(), CV_32F);
    multiply(img, w, imgw);
    cv::Mat w0 = w.clone();
    cv::Mat r0 = cv::Mat::ones(w.size(), CV_32F);
    copyMakeBorder(w0, w0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    copyMakeBorder(r0, r0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(r0), GetMidRow(w0)}, "1DWindows", "x", "window", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory + "/1DWindows.png");
    // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(Fourier::fftlogmagn(r0)), GetMidRow(Fourier::fftlogmagn(w0))}, "1DWindowsDFT", "fx", "log DFT", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory
    // +
    // "/1DWindowsDFT.png");

    // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImage.png");
    Plot2D::Plot("IPCdebug2D", img);

    // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImageWindow.png");
    Plot2D::Plot("IPCdebug2D", imgw);

    // Plot2D::Plot(Fourier::fftlogmagn(r0), "2DWindowDFTR", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTR.png");
    // Plot2D::Plot(Fourier::fftlogmagn(w0), "2DWindowDFTH", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTH.png");
    // Plot2D::Plot(w, "2DWindow", "x", "y", "window", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindow.png");
    // Plot2D::Plot(Fourier::fftlogmagn(img), "2DImageDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageDFT.png");
    // Plot2D::Plot(Fourier::fftlogmagn(imgw), "2DImageWindowDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageWindowDFT.png");
  }

  if constexpr (debugBandpass)
  {
    cv::Mat bpR = cv::Mat::zeros(mRows, mCols, CV_32F);
    cv::Mat bpG = cv::Mat::zeros(mRows, mCols, CV_32F);
    for (i32 r = 0; r < mRows; ++r)
    {
      for (i32 c = 0; c < mCols; ++c)
      {
        bpR.at<f32>(r, c) = BandpassREquation(r, c);

        if (mBandpassL <= 0 and mBandpassH < 1)
          bpG.at<f32>(r, c) = LowpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH >= 1)
          bpG.at<f32>(r, c) = HighpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH < 1)
          bpG.at<f32>(r, c) = BandpassGEquation(r, c);
      }
    }

    if (mBandpassL > 0 and mBandpassH < 1)
      normalize(bpG, bpG, 0.0, 1.0, cv::NORM_MINMAX);

    cv::Mat bpR0, bpG0;
    copyMakeBorder(bpR, bpR0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    copyMakeBorder(bpG, bpG0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(bpR0), GetMidRow(bpG0)}, "b0", "x", "filter", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory + "/1DBandpass.png");
    // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(Fourier::ifftlogmagn(bpR0)), GetMidRow(Fourier::ifftlogmagn(bpG0))}, "b1", "fx", "log IDFT", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory
    // +
    // "/1DBandpassIDFT.png"); Plot2D::Plot(bpR, "b2", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassR.png"); Plot2D::Plot(bpG, "b3", "x", "y", "filter", 0, 1, 0, 1, 0,
    // mDebugDirectory + "/2DBandpassG.png"); Plot2D::Plot(Fourier::ifftlogmagn(bpR0, 10), "b4", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassRIDFT.png");
    // Plot2D::Plot(Fourier::ifftlogmagn(bpG0, 10), "b5", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassGIDFT.png");
  }

  if constexpr (debugBandpassRinging)
  {
    cv::Mat img = roicrop(loadImage("../resources/test.png"), 4098 / 2, 4098 / 2, mCols, mRows);
    cv::Mat fftR = Fourier::fft(img);
    cv::Mat fftG = Fourier::fft(img);
    cv::Mat filterR = cv::Mat::zeros(img.size(), CV_32F);
    cv::Mat filterG = cv::Mat::zeros(img.size(), CV_32F);

    for (i32 r = 0; r < mRows; ++r)
    {
      for (i32 c = 0; c < mCols; ++c)
      {
        filterR.at<f32>(r, c) = BandpassREquation(r, c);

        if (mBandpassL <= 0 and mBandpassH < 1)
          filterG.at<f32>(r, c) = LowpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH >= 1)
          filterG.at<f32>(r, c) = HighpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH < 1)
          filterG.at<f32>(r, c) = BandpassGEquation(r, c);
      }
    }

    Fourier::ifftshift(filterR);
    Fourier::ifftshift(filterG);

    cv::Mat filterRc = Fourier::dupchansc(filterR);
    cv::Mat filterGc = Fourier::dupchansc(filterG);

    multiply(fftR, filterRc, fftR);
    multiply(fftG, filterGc, fftG);

    cv::Mat imgfR = Fourier::ifft(fftR);
    cv::Mat imgfG = Fourier::ifft(fftG);

    normalize(imgfR, imgfR, 0.0, 1.0, cv::NORM_MINMAX);
    normalize(imgfG, imgfG, 0.0, 1.0, cv::NORM_MINMAX);

    // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DBandpassImageR.png");
    Plot2D::Plot("IPCdebug2D", imgfR);

    // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DBandpassImageG.png");
    Plot2D::Plot("IPCdebug2D", imgfG);
  }

  LOG_INFO("IPC debug stuff shown");
}
catch (const std::exception& e)
{
  LOG_ERROR("IterativePhaseCorrelation::ShowDebugStuff() error: {}", e.what());
}
catch (...)
{
  LOG_ERROR("IterativePhaseCorrelation::ShowDebugStuff() error: {}", "Unknown error");
}