#include "IPCDebug.hpp"
#include "IPC.hpp"
#include "ImageProcessing/Noise.hpp"

void IPCDebug::DebugInputImages(const IPC& ipc, const cv::Mat& image1, const cv::Mat& image2)
{
  Plot::Plot({.name = fmt::format("{} I1", ipc.mDebugName), .z = image1, .cmap = "gray"});
  Plot::Plot({.name = fmt::format("{} I2", ipc.mDebugName), .z = image2, .cmap = "gray"});
}

void IPCDebug::DebugFourierTransforms(const IPC& ipc, const cv::Mat& dft1, const cv::Mat& dft2)
{
  auto plot1 = dft1.clone();
  FFTShift(plot1);
  Plot::Plot(fmt::format("{} DFT1lm", ipc.mDebugName), LogMagnitude(plot1));
  Plot::Plot(fmt::format("{} DFT1p", ipc.mDebugName), Phase(plot1));

  auto plot2 = dft2.clone();
  FFTShift(plot2);
  Plot::Plot(fmt::format("{} DFT2lm", ipc.mDebugName), LogMagnitude(plot2));
  Plot::Plot(fmt::format("{} DFT2p", ipc.mDebugName), Phase(plot2));
}

void IPCDebug::DebugCrossPowerSpectrum(const IPC& ipc, const cv::Mat& crosspower)
{
  Plot::Plot({.name = fmt::format("{} CP magnitude", ipc.mDebugName), .z = FFTShift(Magnitude(crosspower)), .cmap = "jet"});
  Plot::Plot({.name = fmt::format("{} CP Phase", ipc.mDebugName), .z = FFTShift(Phase(crosspower)), .cmap = "jet"});
}

void IPCDebug::DebugL3(const IPC& ipc, const cv::Mat& L3)
{
  Plot::Plot({.name = fmt::format("{} L3", ipc.mDebugName), .z = L3, .cmap = "jet"});
}

void IPCDebug::DebugL2(const IPC& ipc, const cv::Mat& L2)
{
  auto plot = L2.clone();
  cv::resize(plot, plot, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  Plot::Plot(
      {.name = fmt::format("{} L2", ipc.mDebugName), .z = plot, .xmin = 0, .xmax = static_cast<f32>(L2.cols - 1), .ymin = 0, .ymax = static_cast<f32>(L2.rows - 1), .cmap = "jet"});
}

void IPCDebug::DebugL2U(const IPC& ipc, const cv::Mat& L2, const cv::Mat& L2U)
{
  Plot::Plot({.name = fmt::format("{} L2U", ipc.mDebugName),
      .savepath = not ipc.mDebugDirectory.empty() ? fmt::format("{}/L2U_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : "",
      .z = L2U,
      .cmap = "jet"});

  if (false)
  {
    cv::Mat nearest, linear, cubic;
    cv::resize(L2, nearest, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
    cv::resize(L2, linear, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_LINEAR);
    cv::resize(L2, cubic, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_CUBIC);

    Plot::Plot({.name = fmt::format("{} L2U nearest", ipc.mDebugName), .z = nearest, .cmap = "jet"});
    Plot::Plot({.name = fmt::format("{} L2U linear", ipc.mDebugName), .z = linear, .cmap = "jet"});
    Plot::Plot({.name = fmt::format("{} L2U cubic", ipc.mDebugName), .z = cubic, .cmap = "jet"});
  }
}

void IPCDebug::DebugL1B(const IPC& ipc, const cv::Mat& L2U, i32 L1size, const cv::Point2d& L3shift, f64 UC)
{
  cv::Mat mat = IPC::CalculateL1(L2U, cv::Point(L2U.cols / 2, L2U.rows / 2), L1size).clone();
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
  mat = mat.mul(IPC::GetL1Window(ipc.mL1WinT, mat.rows));
  DrawCrosshairs(mat);
  if (ipc.mDebugTrueShift != ipc.mDefaultDebugTrueShift)
    DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + UC * (ipc.mDebugTrueShift - L3shift));
  cv::resize(mat, mat, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  Plot::Plot({.name = fmt::format("{} L1B", ipc.mDebugName),
      .savepath = not ipc.mDebugDirectory.empty() ? fmt::format("{}/L1B_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : "",
      .z = mat,
      .cmap = "jet"});
}

void IPCDebug::DebugL1A(const IPC& ipc, const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, f64 UC, bool last)
{
  cv::Mat mat = L1.clone();
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
  mat = mat.mul(IPC::GetL1Window(ipc.mL1WinT, mat.rows));
  DrawCrosshairs(mat);
  if (ipc.mDebugTrueShift != ipc.mDefaultDebugTrueShift)
    DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + UC * (ipc.mDebugTrueShift - L3shift) - L2Ushift);
  // cv::resize(mat, mat, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  Plot::Plot({.name = fmt::format("{} L1A", ipc.mDebugName),
      .savepath = not ipc.mDebugDirectory.empty() and last ? fmt::format("{}/L1A_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : "",
      .z = mat,
      .xmin = 0,
      .xmax = static_cast<f32>(L1.cols - 1),
      .ymin = 0,
      .ymax = static_cast<f32>(L1.rows - 1),
      .cmap = "jet"});
}

void IPCDebug::DebugShift(const IPC& ipc, f64 maxShift, f64 noiseStdev)
{
  const auto image = LoadUnitFloatImage<IPC::Float>(GetProjectDirectoryPath("data/debug/star.png"));
  cv::Point2d shift(Random::Rand(-1., 1.) * maxShift, Random::Rand(-1., 1.) * maxShift);
  // cv::Point2d shift(maxShift, -maxShift);

  cv::Point2i point(std::clamp(Random::Rand() * image.cols, static_cast<f64>(ipc.mCols), static_cast<f64>(image.cols - ipc.mCols)),
      std::clamp(Random::Rand() * image.rows, static_cast<f64>(ipc.mRows), static_cast<f64>(image.rows - ipc.mRows)));
  // cv::Point2i point(std::clamp(0.45 * image.cols, static_cast<f64>(ipc.mCols), static_cast<f64>(image.cols - ipc.mCols)),
  //     std::clamp(0.5 * image.rows, static_cast<f64>(ipc.mRows), static_cast<f64>(image.rows - ipc.mRows)));

  cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
  cv::Mat imageShifted;
  cv::warpAffine(image, imageShifted, Tmat, image.size());
  cv::Mat image1 = RoiCrop(image, point.x, point.y, ipc.mCols, ipc.mRows);
  cv::Mat image2 = RoiCrop(imageShifted, point.x, point.y, ipc.mCols, ipc.mRows);
  ipc.SetDebugTrueShift(shift);
  AddNoise<IPC::Float>(image1, noiseStdev);
  AddNoise<IPC::Float>(image2, noiseStdev);

  const auto ipcshift = ipc.Calculate<IPC::Mode::Debug>(image1, image2);
  const auto error = ipcshift - shift;

  LOG_INFO("Artificial shift = [{:.4f}, {:.4f}]", shift.x, shift.y);
  LOG_INFO("Estimated shift = [{:.4f}, {:.4f}]", ipcshift.x, ipcshift.y);
  LOG_INFO("Error = [{:.4f}, {:.4f}]", error.x, error.y);
}

void IPCDebug::DebugShift2(const IPC& ipc, const std::string& image1Path, const std::string& image2Path, f64 noiseStdev)
{
  LOG_FUNCTION;
  auto image1 = LoadUnitFloatImage<IPC::Float>(image1Path);
  auto image2 = LoadUnitFloatImage<IPC::Float>(image2Path);
  cv::resize(image1, image1, cv::Size(ipc.GetCols(), ipc.GetRows()));
  cv::resize(image2, image2, cv::Size(ipc.GetCols(), ipc.GetRows()));
  AddNoise<IPC::Float>(image1, noiseStdev);
  AddNoise<IPC::Float>(image2, noiseStdev);
  const auto ipcshift = ipc.Calculate<IPC::Mode::Debug>(image1, image2);
  LOG_INFO("Estimated shift = [{:.4f}, {:.4f}]", ipcshift.x, ipcshift.y);
}

void IPCDebug::DebugAlign(const IPC& ipc, const std::string& image1Path, const std::string& image2Path, f64 noiseStdev)
{
  LOG_FUNCTION;
  static constexpr bool save = false;
  static constexpr bool artificial = false;
  static constexpr bool create = false;
  ipc.SetDebugDirectory(GetProjectDirectoryPath("data/debug").string());

  if constexpr (create) // create test images
  {
    auto image = LoadUnitFloatImage<IPC::Float>(GetProjectDirectoryPath("data/debug/304A.png"));
    auto imagee = LoadUnitFloatImage<IPC::Float>(GetProjectDirectoryPath("data/debug/171A.png"));

    image.convertTo(image, CV_8U, 255);
    imagee.convertTo(imagee, CV_8U, 255);

    const auto shift = cv::Point2d(0.256 * ipc.mCols, -0.232 * ipc.mRows);
    const auto scale = 1.43;
    const auto rotation = 45;

    const auto image1 = RoiCropMid(image, ipc.mCols, ipc.mRows);
    Shift(imagee, shift);
    Rotate(imagee, rotation, scale);
    const auto image2 = RoiCropMid(imagee, ipc.mCols, ipc.mRows);

    Plot::Plot("image1", image1);
    Plot::Plot("image2", image2);

    cv::imwrite(GetProjectDirectoryPath("data/debug/artificialx1.png").string(), image1);
    cv::imwrite(GetProjectDirectoryPath("data/debug/artificialx2.png").string(), image2);
    return;
  }

  auto image1 = LoadUnitFloatImage<IPC::Float>(image1Path);
  auto image2 = LoadUnitFloatImage<IPC::Float>(image2Path);

  if (image1.rows != image1.cols) // crop images to be square
  {
    const auto size = std::min({image1.rows, image1.cols, image2.rows, image2.cols});
    image1 = RoiCropMid(image1, size, size);
    image2 = RoiCropMid(image2, size, size);
  }

  if (image1.rows != ipc.GetRows() or image1.cols != ipc.GetCols())
  {
    cv::resize(image1, image1, cv::Size(ipc.GetCols(), ipc.GetRows()));
    cv::resize(image2, image2, cv::Size(ipc.GetCols(), ipc.GetRows()));
  }

  if constexpr (save)
  {
    Plot::Plot("../debug/input1.png", image1);
    Plot::Plot("../debug/input2.png", image2);
  }

  AddNoise<IPC::Float>(image1, noiseStdev);
  AddNoise<IPC::Float>(image2, noiseStdev);

  if constexpr (artificial)
  {
    const auto shift = cv::Point2d(-0.176 * ipc.GetCols(), 0.132 * ipc.GetRows());
    const auto scale = 1.2;
    const auto rotation = 70;
    Shift(image2, shift);
    Rotate(image2, rotation, scale);
    LOG_DEBUG("Artificial shift: {}", shift);
    LOG_DEBUG("Artificial rotation/scale: {}/{}", rotation, scale);
  }

  IPCAlign::Align(ipc, image1, image2);
}

void IPCDebug::DebugGradualShift(const IPC& ipc, f64 maxShift, f64 noiseStdev)
{
  LOG_FUNCTION;
  ipc.SetDebugDirectory("../debug/peakshift");
  const cv::Mat image1 = LoadUnitFloatImage<IPC::Float>("../debug/AIA/171A.png");
  cv::Mat crop1 = RoiCropMid(image1, ipc.mCols, ipc.mRows);
  crop1 += GetNoise<IPC::Float>(crop1.size(), noiseStdev);
  cv::Mat image2 = image1.clone();
  cv::Mat crop2;
  const i32 iters = 51;
  cv::Mat noise2 = GetNoise<IPC::Float>(crop1.size(), noiseStdev);

  for (i32 i = 0; i < iters; ++i)
  {
    ipc.SetDebugIndex(i);
    const cv::Point2d shift(maxShift * i / (iters - 1), maxShift * i / (iters - 1));
    const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
    cv::warpAffine(image1, image2, Tmat, image2.size());
    crop2 = RoiCropMid(image2, ipc.mCols, ipc.mRows) + noise2;
    ipc.SetDebugTrueShift(shift);
    const auto ipcshift = ipc.Calculate(crop1, crop2);
    LOG_INFO("Artificial shift = {} / Estimated shift = {} / Error = {}", shift, ipcshift, ipcshift - shift);
  }
}

void IPCDebug::DebugWindow(const IPC& ipc)
{
  cv::Mat img = RoiCrop(LoadUnitFloatImage<IPC::Float>("../data/test.png"), 2048, 2048, ipc.mCols, ipc.mRows);
  cv::Mat w, imgw;
  cv::createHanningWindow(w, img.size(), GetMatType<IPC::Float>());
  cv::multiply(img, w, imgw);
  cv::Mat w0 = w.clone();
  cv::Mat r0 = cv::Mat::ones(w.size(), GetMatType<IPC::Float>());
  cv::copyMakeBorder(w0, w0, 5, 5, 5, 5, cv::BORDER_CONSTANT, cv::Scalar::all(0));
  cv::copyMakeBorder(r0, r0, 5, 5, 5, 5, cv::BORDER_CONSTANT, cv::Scalar::all(0));

  // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(r0), GetMidRow(w0)}, "1DWindows", "x", "window", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory
  // + "/1DWindows.png"); Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(FFTLogMagnitude(r0)), GetMidRow(FFTLogMagnitude(w0))},
  // "1DWindowsDFT", "fx", "log DFT", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory
  // +
  // "/1DWindowsDFT.png");

  // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImage.png");
  // Plot2D::Plot("IPCdebug2D", img);

  // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImageWindow.png");
  // Plot2D::Plot("IPCdebug2D", imgw);

  // Plot2D::Plot(FFTLogMagnitude(r0), "2DWindowDFTR", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTR.png");
  // Plot2D::Plot(FFTLogMagnitude(w0), "2DWindowDFTH", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTH.png");
  // Plot2D::Plot(w, "2DWindow", "x", "y", "window", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindow.png");
  // Plot2D::Plot(FFTLogMagnitude(img), "2DImageDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageDFT.png");
  // Plot2D::Plot(FFTLogMagnitude(imgw), "2DImageWindowDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageWindowDFT.png");
}

void IPCDebug::DebugBandpass(const IPC& ipc)
{
  cv::Mat bpR = cv::Mat(ipc.mRows, ipc.mCols, GetMatType<IPC::Float>());
  cv::Mat bpG = cv::Mat(ipc.mRows, ipc.mCols, GetMatType<IPC::Float>());
  for (i32 r = 0; r < ipc.mRows; ++r)
  {
    for (i32 c = 0; c < ipc.mCols; ++c)
    {
      bpR.at<IPC::Float>(r, c) = ipc.BandpassREquation(r, c);

      if (ipc.mBPL <= 0 and ipc.mBPH < 1)
        bpG.at<IPC::Float>(r, c) = ipc.LowpassEquation(r, c);
      else if (ipc.mBPL > 0 and ipc.mBPH >= 1)
        bpG.at<IPC::Float>(r, c) = ipc.HighpassEquation(r, c);
      else if (ipc.mBPL > 0 and ipc.mBPH < 1)
        bpG.at<IPC::Float>(r, c) = ipc.BandpassGEquation(r, c);
    }
  }

  if (ipc.mBPL > 0 and ipc.mBPH < 1)
    cv::normalize(bpG, bpG, 0.0, 1.0, cv::NORM_MINMAX);

  cv::Mat bpR0, bpG0;
  cv::copyMakeBorder(bpR, bpR0, 5, 5, 5, 5, cv::BORDER_CONSTANT, cv::Scalar::all(0));
  cv::copyMakeBorder(bpG, bpG0, 5, 5, 5, 5, cv::BORDER_CONSTANT, cv::Scalar::all(0));

  // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(bpR0), GetMidRow(bpG0)}, "b0", "x", "filter", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory
  // + "/1DBandpass.png"); Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(IFFTLogMagnitude(bpR0)), GetMidRow(IFFTLogMagnitude(bpG0))}, "b1",
  // "fx", "log IDFT", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory
  // +
  // "/1DBandpassIDFT.png"); Plot2D::Plot(bpR, "b2", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassR.png"); Plot2D::Plot(bpG,
  // "b3", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassG.png"); Plot2D::Plot(IFFTLogMagnitude(bpR0, 10), "b4", "fx", "fy",
  // "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassRIDFT.png"); Plot2D::Plot(IFFTLogMagnitude(bpG0, 10), "b5", "fx", "fy", "log
  // IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassGIDFT.png");
}

void IPCDebug::DebugBandpassRinging(const IPC& ipc)
{
  cv::Mat img = RoiCrop(LoadUnitFloatImage<IPC::Float>("../data/test.png"), 4098 / 2, 4098 / 2, ipc.mCols, ipc.mRows);
  cv::Mat fftR = FFT(img);
  cv::Mat fftG = FFT(img);
  cv::Mat filterR = cv::Mat(img.size(), GetMatType<IPC::Float>());
  cv::Mat filterG = cv::Mat(img.size(), GetMatType<IPC::Float>());

  for (i32 r = 0; r < ipc.mRows; ++r)
  {
    for (i32 c = 0; c < ipc.mCols; ++c)
    {
      filterR.at<IPC::Float>(r, c) = ipc.BandpassREquation(r, c);

      if (ipc.mBPL <= 0 and ipc.mBPH < 1)
        filterG.at<IPC::Float>(r, c) = ipc.LowpassEquation(r, c);
      else if (ipc.mBPL > 0 and ipc.mBPH >= 1)
        filterG.at<IPC::Float>(r, c) = ipc.HighpassEquation(r, c);
      else if (ipc.mBPL > 0 and ipc.mBPH < 1)
        filterG.at<IPC::Float>(r, c) = ipc.BandpassGEquation(r, c);
    }
  }

  IFFTShift(filterR);
  IFFTShift(filterG);

  cv::Mat filterRc = DuplicateChannelsCopy(filterR);
  cv::Mat filterGc = DuplicateChannelsCopy(filterG);

  cv::multiply(fftR, filterRc, fftR);
  cv::multiply(fftG, filterGc, fftG);

  cv::Mat imgfR = IFFT(fftR);
  cv::Mat imgfG = IFFT(fftG);

  cv::normalize(imgfR, imgfR, 0.0, 1.0, cv::NORM_MINMAX);
  cv::normalize(imgfG, imgfG, 0.0, 1.0, cv::NORM_MINMAX);

  // Plot2D::SetSavePath("IPCdebug2D", ipc.mDebugDirectory + "/2DBandpassImageR.png");
  // Plot2D::Plot("IPCdebug2D", imgfR);

  // Plot2D::SetSavePath("IPCdebug2D", ipc.mDebugDirectory + "/2DBandpassImageG.png");
  // Plot2D::Plot("IPCdebug2D", imgfG);
}
