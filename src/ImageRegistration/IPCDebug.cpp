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
}

void IPCDebug::DebugL2(const IPC& ipc, const cv::Mat& L2)
{
  auto plot = L2.clone();
  cv::resize(plot, plot, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L2", ipc.mDebugName), {.z = plot});
}

void IPCDebug::DebugL2U(const IPC& ipc, const cv::Mat& L2, const cv::Mat& L2U)
{
  PyPlot::PlotSurf(fmt::format("{} L2U surf", ipc.mDebugName), {.z = L2U, .save = not ipc.mDebugDirectory.empty() ? fmt::format("{}/L2U_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : ""});

  if (0)
  {
    cv::Mat nearest, linear, cubic;
    cv::resize(L2, nearest, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
    cv::resize(L2, linear, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_LINEAR);
    cv::resize(L2, cubic, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_CUBIC);

    PyPlot::Plot("IPCL2UN", {.z = nearest});
    PyPlot::Plot("IPCL2UL", {.z = linear});
    PyPlot::Plot("IPCL2UC", {.z = cubic});
  }
}

void IPCDebug::DebugL1B(const IPC& ipc, const cv::Mat& L2U, i32 L1size, const cv::Point2d& L3shift, f64 UC)
{
  cv::Mat mat = IPC::CalculateL1(L2U, cv::Point(L2U.cols / 2, L2U.rows / 2), L1size).clone();
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
  mat = mat.mul(IPC::GetL1Window(ipc.mL1WinT, mat.rows));
  DrawCrosshairs(mat);
  DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + UC * (ipc.mDebugTrueShift - L3shift));
  cv::resize(mat, mat, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L1B", ipc.mDebugName), {.z = mat, .save = not ipc.mDebugDirectory.empty() ? fmt::format("{}/L1B_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : ""});
}

void IPCDebug::DebugL1A(const IPC& ipc, const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, f64 UC, bool last)
{
  cv::Mat mat = L1.clone();
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
  mat = mat.mul(IPC::GetL1Window(ipc.mL1WinT, mat.rows));
  DrawCrosshairs(mat);
  DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + UC * (ipc.mDebugTrueShift - L3shift) - L2Ushift);
  cv::resize(mat, mat, {ipc.mL2Usize, ipc.mL2Usize}, 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L1A", ipc.mDebugName), {.z = mat, .save = not ipc.mDebugDirectory.empty() and last ? fmt::format("{}/L1A_{}.png", ipc.mDebugDirectory, ipc.mDebugIndex) : ""});
}

void IPCDebug::ShowDebugStuff(const IPC& ipc)
try
{
  LOG_FUNCTION("ShowDebugStuff()");

  constexpr bool debugShift = true;
  constexpr bool debugAlign = false;
  constexpr bool debugGradualShift = false;
  constexpr bool debugWindow = false;
  constexpr bool debugBandpass = false;
  constexpr bool debugBandpassRinging = false;
  constexpr f64 noiseStdev = 0.01;

  if constexpr (debugShift)
  {
    const auto image = LoadUnitFloatImage<IPC::Float>("../debug/AIA/171A.png");
    const f64 shiftmax = 0.4 * ipc.mRows;
    cv::Point2d shift(Random::Rand(-1., 1.) * shiftmax, Random::Rand(-1., 1.) * shiftmax);
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

    const auto ipcshift = ipc.Calculate<true>(image1, image2);

    LOG_INFO("Artificial shift = {}", shift);
    LOG_INFO("Estimated shift = {}", ipcshift);
    LOG_INFO("Error = {}", ipcshift - shift);
  }

  if constexpr (debugAlign)
  {
    auto image1 = LoadUnitFloatImage<IPC::Float>("../debug/AIA/304A.png");
    auto image2 = LoadUnitFloatImage<IPC::Float>("../debug/AIA/171A.png");
    if (ipc.mRows != image1.rows or ipc.mCols != image1.cols)
    {
      image1 = RoiCrop(image1, image1.cols / 2, image1.rows / 2, ipc.mCols, ipc.mRows);
      image2 = RoiCrop(image2, image1.cols / 2, image1.rows / 2, ipc.mCols, ipc.mRows);
    }

    Shift(image2, -950, 1050);
    Rotate(image2, 70, 1.2);

    AddNoise<IPC::Float>(image1, noiseStdev);
    AddNoise<IPC::Float>(image2, noiseStdev);

    const auto aligned = IPCAlign::Align(ipc, image1, image2);
  }

  if constexpr (debugGradualShift)
  {
    ipc.SetDebugDirectory("../debug/peakshift");
    const cv::Mat image1 = LoadUnitFloatImage<IPC::Float>("../debug/AIA/171A.png");
    const cv::Mat crop1 = RoiCropMid(image1, ipc.mCols, ipc.mRows) + GetNoise<IPC::Float>(crop1.size(), noiseStdev);
    cv::Mat image2 = image1.clone();
    cv::Mat crop2;
    const i32 iters = 51;
    const f64 totalshift = 1.;
    cv::Mat noise2 = GetNoise<IPC::Float>(crop1.size(), noiseStdev);

    for (i32 i = 0; i < iters; ++i)
    {
      ipc.SetDebugIndex(i);
      const cv::Point2d shift(totalshift * i / (iters - 1), totalshift * i / (iters - 1));
      const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::warpAffine(image1, image2, Tmat, image2.size());
      crop2 = RoiCropMid(image2, ipc.mCols, ipc.mRows) + noise2;
      ipc.SetDebugTrueShift(shift);
      const auto ipcshift = ipc.Calculate<true>(crop1, crop2);
      LOG_INFO("Artificial shift = {} / Estimated shift = {} / Error = {}", shift, ipcshift, ipcshift - shift);
    }
  }

  if constexpr (debugWindow)
  {
    cv::Mat img = RoiCrop(LoadUnitFloatImage<IPC::Float>("../data/test.png"), 2048, 2048, ipc.mCols, ipc.mRows);
    cv::Mat w, imgw;
    cv::createHanningWindow(w, img.size(), GetMatType<IPC::Float>());
    cv::multiply(img, w, imgw);
    cv::Mat w0 = w.clone();
    cv::Mat r0 = cv::Mat::ones(w.size(), GetMatType<IPC::Float>());
    cv::copyMakeBorder(w0, w0, 5, 5, 5, 5, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::copyMakeBorder(r0, r0, 5, 5, 5, 5, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(r0), GetMidRow(w0)}, "1DWindows", "x", "window", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory + "/1DWindows.png");
    // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(Fourier::fftlogmagn(r0)), GetMidRow(Fourier::fftlogmagn(w0))}, "1DWindowsDFT", "fx", "log DFT", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory
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

  if constexpr (debugBandpass)
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

    // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(bpR0), GetMidRow(bpG0)}, "b0", "x", "filter", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory + "/1DBandpass.png");
    // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(Fourier::ifftlogmagn(bpR0)), GetMidRow(Fourier::ifftlogmagn(bpG0))}, "b1", "fx", "log IDFT", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory
    // +
    // "/1DBandpassIDFT.png"); Plot2D::Plot(bpR, "b2", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassR.png"); Plot2D::Plot(bpG, "b3", "x", "y", "filter", 0, 1, 0, 1, 0,
    // mDebugDirectory + "/2DBandpassG.png"); Plot2D::Plot(Fourier::ifftlogmagn(bpR0, 10), "b4", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassRIDFT.png");
    // Plot2D::Plot(Fourier::ifftlogmagn(bpG0, 10), "b5", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassGIDFT.png");
  }

  if constexpr (debugBandpassRinging)
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

  LOG_INFO("IPC debug stuff shown");
}
catch (const std::exception& e)
{
  LOG_ERROR("ShowDebugStuff() error: {}", e.what());
}
