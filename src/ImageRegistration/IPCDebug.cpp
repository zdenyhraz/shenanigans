#include "IPCDebug.hpp"
#include "IPC.hpp"
#include "Filtering/Noise.hpp"
#include "UtilsCV/Vectmat.hpp"

void IPCDebug::DebugInputImages(const IPC& ipc, const cv::Mat& image1, const cv::Mat& image2)
{
  PyPlot::Plot(fmt::format("{} I1", ipc.mDebugName), {.z = image1, .cmap = "gray"});
  PyPlot::Plot(fmt::format("{} I2", ipc.mDebugName), {.z = image2, .cmap = "gray"});
}

void IPCDebug::DebugFourierTransforms(const IPC& ipc, const cv::Mat& dft1, const cv::Mat& dft2)
{
  auto plot1 = dft1.clone();
  Fourier::fftshift(plot1);
  PyPlot::Plot(fmt::format("{} DFT1lm", ipc.mDebugName), {.z = Fourier::logmagn(plot1)});
  PyPlot::Plot(fmt::format("{} DFT1p", ipc.mDebugName), {.z = Fourier::phase(plot1)});

  auto plot2 = dft2.clone();
  Fourier::fftshift(plot2);
  PyPlot::Plot(fmt::format("{} DFT2lm", ipc.mDebugName), {.z = Fourier::logmagn(plot2)});
  PyPlot::Plot(fmt::format("{} DFT2p", ipc.mDebugName), {.z = Fourier::phase(plot2)});
}

void IPCDebug::DebugCrossPowerSpectrum(const IPC& ipc, const cv::Mat& crosspower)
{
  PyPlot::Plot(fmt::format("{} CP magnitude", ipc.mDebugName), {.z = Fourier::fftshift(Fourier::magn(crosspower))});
  PyPlot::Plot(fmt::format("{} CP phase", ipc.mDebugName), {.z = Fourier::fftshift(Fourier::phase(crosspower))});
}

void IPCDebug::DebugL3(const IPC& ipc, const cv::Mat& L3)
{
  PyPlot::Plot(fmt::format("{} L3", ipc.mDebugName), {.z = L3});
  // PyPlot::PlotSurf(fmt::format("{} L3 surf", ipc.mDebugName), {.z = L3});
}

void IPCDebug::DebugL2(const IPC& ipc, const cv::Mat& L2)
{
  auto plot = L2.clone();
  cv::resize(plot, plot, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L2", ipc.mDebugName), {.z = plot});
}

void IPCDebug::DebugL2U(const IPC& ipc, const cv::Mat& L2, const cv::Mat& L2U)
{
  PyPlot::Plot(fmt::format("{} L2U", ipc.mDebugName),
      {.z = L2U, .save = not ipc.mDebugDirectory.empty() ? fmt::format("{}/L2U_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : ""});

  PyPlot::PlotSurf(fmt::format("{} L2U surf", ipc.mDebugName),
      {.z = L2U, .save = not ipc.mDebugDirectory.empty() ? fmt::format("{}/L2Us_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : ""});

  if (false)
  {
    cv::Mat nearest, linear, cubic;
    cv::resize(L2, nearest, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
    cv::resize(L2, linear, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_LINEAR);
    cv::resize(L2, cubic, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_CUBIC);

    PyPlot::Plot(fmt::format("{} L2U nearest", ipc.mDebugName), {.z = nearest});
    PyPlot::Plot(fmt::format("{} L2U linear", ipc.mDebugName), {.z = linear});
    PyPlot::Plot(fmt::format("{} L2U cubic", ipc.mDebugName), {.z = cubic});

    PyPlot::PlotSurf(fmt::format("{} L2U nearest surf", ipc.mDebugName), {.z = nearest});
    PyPlot::PlotSurf(fmt::format("{} L2U linear surf", ipc.mDebugName), {.z = linear});
    PyPlot::PlotSurf(fmt::format("{} L2U cubic surf", ipc.mDebugName), {.z = cubic});
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
  PyPlot::Plot(fmt::format("{} L1B", ipc.mDebugName),
      {.z = mat, .save = not ipc.mDebugDirectory.empty() ? fmt::format("{}/L1B_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : ""});
}

void IPCDebug::DebugL1A(const IPC& ipc, const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, f64 UC, bool last)
{
  cv::Mat mat = L1.clone();
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
  mat = mat.mul(IPC::GetL1Window(ipc.mL1WinT, mat.rows));
  DrawCrosshairs(mat);
  if (ipc.mDebugTrueShift != ipc.mDefaultDebugTrueShift)
    DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + UC * (ipc.mDebugTrueShift - L3shift) - L2Ushift);
  cv::resize(mat, mat, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L1A", ipc.mDebugName),
      {.z = mat, .save = not ipc.mDebugDirectory.empty() and last ? fmt::format("{}/L1A_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : ""});
}

void IPCDebug::DebugShift(const IPC& ipc, f64 maxShift, f64 noiseStdev)
{
  const auto image = LoadUnitFloatImage<IPC::Float>("../debug/AIA/171A.png");
  cv::Point2d shift(Random::Rand(-1., 1.) * maxShift, Random::Rand(-1., 1.) * maxShift);
  cv::Point2i point(std::clamp(Random::Rand() * image.cols, static_cast<f64>(ipc.mCols), static_cast<f64>(image.cols - ipc.mCols)),
      std::clamp(Random::Rand() * image.rows, static_cast<f64>(ipc.mRows), static_cast<f64>(image.rows - ipc.mRows)));
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
  ipc.SetDebugDirectory("../debug/");

  if constexpr (create) // create test images
  {
    auto image = LoadUnitFloatImage<IPC::Float>("../debug/shapes/shape.png");
    const auto shift = cv::Point2d(0.256 * image.cols, 0.232 * image.rows);
    const auto scale = 1.43;
    const auto rotation = 45;

    Shift(image, -shift);
    cv::Mat image1 = image.clone();
    // AddNoise<IPC::Float>(image1, 0.05);
    Saveimg("../debug/shapes/shape1.png", image1);
    Shift(image, 1.5 * shift);
    Rotate(image, rotation, scale);
    cv::Mat image2 = image.clone();
    // AddNoise<IPC::Float>(image2, 0.05);
    Saveimg("../debug/shapes/shape2.png", image2);
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

  if (false) // histogram equalization
  {
    cv::normalize(image1, image1, 0, 255, cv::NORM_MINMAX);
    cv::normalize(image2, image2, 0, 255, cv::NORM_MINMAX);
    image1.convertTo(image1, CV_8U);
    image2.convertTo(image2, CV_8U);

    if (true) // CLAHE
    {
      auto clahe = cv::createCLAHE();
      clahe->setClipLimit(4);
      clahe->setTilesGridSize({25, 25});
      clahe->apply(image1, image1);
      clahe->apply(image2, image2);
    }
    else // normal
    {
      cv::equalizeHist(image1, image1);
      cv::equalizeHist(image2, image2);
    }

    image1.convertTo(image1, GetMatType<IPC::Float>());
    image2.convertTo(image2, GetMatType<IPC::Float>());
    cv::normalize(image1, image1, 0, 1, cv::NORM_MINMAX);
    cv::normalize(image2, image2, 0, 1, cv::NORM_MINMAX);
  }

  if constexpr (save)
  {
    Saveimg("../debug/input1.png", image1, false, image1.rows >= 256 ? cv::Size(0, 0) : cv::Size(1024, 1024));
    Saveimg("../debug/input2.png", image2, false, image2.rows >= 256 ? cv::Size(0, 0) : cv::Size(1024, 1024));
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
  const cv::Mat crop1 = RoiCropMid(image1, ipc.mCols, ipc.mRows) + GetNoise<IPC::Float>(crop1.size(), noiseStdev);
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
  // + "/1DWindows.png"); Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(Fourier::fftlogmagn(r0)), GetMidRow(Fourier::fftlogmagn(w0))},
  // "1DWindowsDFT", "fx", "log DFT", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory
  // +
  // "/1DWindowsDFT.png");

  // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImage.png");
  // Plot2D::Plot("IPCdebug2D", img);

  // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImageWindow.png");
  // Plot2D::Plot("IPCdebug2D", imgw);

  // Plot2D::Plot(Fourier::fftlogmagn(r0), "2DWindowDFTR", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTR.png");
  // Plot2D::Plot(Fourier::fftlogmagn(w0), "2DWindowDFTH", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTH.png");
  // Plot2D::Plot(w, "2DWindow", "x", "y", "window", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindow.png");
  // Plot2D::Plot(Fourier::fftlogmagn(img), "2DImageDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageDFT.png");
  // Plot2D::Plot(Fourier::fftlogmagn(imgw), "2DImageWindowDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageWindowDFT.png");
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
  // + "/1DBandpass.png"); Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(Fourier::ifftlogmagn(bpR0)), GetMidRow(Fourier::ifftlogmagn(bpG0))}, "b1",
  // "fx", "log IDFT", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory
  // +
  // "/1DBandpassIDFT.png"); Plot2D::Plot(bpR, "b2", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassR.png"); Plot2D::Plot(bpG,
  // "b3", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassG.png"); Plot2D::Plot(Fourier::ifftlogmagn(bpR0, 10), "b4", "fx", "fy",
  // "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassRIDFT.png"); Plot2D::Plot(Fourier::ifftlogmagn(bpG0, 10), "b5", "fx", "fy", "log
  // IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassGIDFT.png");
}

void IPCDebug::DebugBandpassRinging(const IPC& ipc)
{
  cv::Mat img = RoiCrop(LoadUnitFloatImage<IPC::Float>("../data/test.png"), 4098 / 2, 4098 / 2, ipc.mCols, ipc.mRows);
  cv::Mat fftR = Fourier::fft(img);
  cv::Mat fftG = Fourier::fft(img);
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

  Fourier::ifftshift(filterR);
  Fourier::ifftshift(filterG);

  cv::Mat filterRc = Fourier::dupchansc(filterR);
  cv::Mat filterGc = Fourier::dupchansc(filterG);

  cv::multiply(fftR, filterRc, fftR);
  cv::multiply(fftG, filterGc, fftG);

  cv::Mat imgfR = Fourier::ifft(fftR);
  cv::Mat imgfG = Fourier::ifft(fftG);

  cv::normalize(imgfR, imgfR, 0.0, 1.0, cv::NORM_MINMAX);
  cv::normalize(imgfG, imgfG, 0.0, 1.0, cv::NORM_MINMAX);

  // Plot2D::SetSavePath("IPCdebug2D", ipc.mDebugDirectory + "/2DBandpassImageR.png");
  // Plot2D::Plot("IPCdebug2D", imgfR);

  // Plot2D::SetSavePath("IPCdebug2D", ipc.mDebugDirectory + "/2DBandpassImageG.png");
  // Plot2D::Plot("IPCdebug2D", imgfG);
}
