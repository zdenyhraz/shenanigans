#include "IPCDebug.hpp"
#include "IPC.hpp"
#include "ImageProcessing/Noise.hpp"
#include "Utils/Draw.hpp"
#include "Utils/Load.hpp"
#include "Math/Transform.hpp"
#include "Plot/Plot.hpp"
#include "Math/Statistics.hpp"

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
  Plot::Plot({.name = fmt::format("{} L2", ipc.mDebugName),
      .z = plot,
      .xmin = 0,
      .xmax = static_cast<float>(L2.cols - 1),
      .ymin = 0,
      .ymax = static_cast<float>(L2.rows - 1),
      .cmap = "jet"});
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

void IPCDebug::DebugL1B(const IPC& ipc, const cv::Mat& L2U, int L1size, const cv::Point2d& L3shift, double UC)
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

void IPCDebug::DebugL1A(const IPC& ipc, const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, double UC, bool last)
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
      .xmax = static_cast<float>(L1.cols - 1),
      .ymin = 0,
      .ymax = static_cast<float>(L1.rows - 1),
      .cmap = "jet"});
}

void IPCDebug::DebugShift(const IPC& ipc, double maxShift, double noiseStdev)
{
  const auto image = LoadUnitFloatImage<IPC::Float>(GetProjectPath("data/debug/star.png"));
  cv::Point2d shift(Random::Rand(-1., 1.) * maxShift, Random::Rand(-1., 1.) * maxShift);
  cv::Point2i point(std::clamp(Random::Rand() * image.cols, static_cast<double>(ipc.mCols), static_cast<double>(image.cols - ipc.mCols)),
      std::clamp(Random::Rand() * image.rows, static_cast<double>(ipc.mRows), static_cast<double>(image.rows - ipc.mRows)));

  cv::Mat Tmat = (cv::Mat_<double>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
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

void IPCDebug::DebugShift2(const IPC& ipc, const std::string& image1Path, const std::string& image2Path, double noiseStdev)
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

void IPCDebug::DebugAlign(const IPC& ipc, const std::string& image1Path, const std::string& image2Path, double noiseStdev)
{
  LOG_FUNCTION;
  static constexpr bool save = false;
  static constexpr bool artificial = false;
  static constexpr bool create = false;
  ipc.SetDebugDirectory(GetProjectPath("data/debug").string());

  if constexpr (create) // create test images
  {
    auto image = LoadUnitFloatImage<IPC::Float>(GetProjectPath("data/debug/304A.png"));
    auto imagee = LoadUnitFloatImage<IPC::Float>(GetProjectPath("data/debug/171A.png"));

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

    cv::imwrite(GetProjectPath("data/debug/artificialx1.png").string(), image1);
    cv::imwrite(GetProjectPath("data/debug/artificialx2.png").string(), image2);
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

void IPCDebug::DebugGradualShift(const IPC& ipc, double maxShift, double noiseStdev)
{
  LOG_FUNCTION;
  cv::Mat image1 = cv::Mat::zeros(ipc.GetRows() * 2, ipc.GetCols() * 2, CV_64F);
  cv::rectangle(image1, cv::Point(ipc.GetRows() * 1.1, ipc.GetRows() * 0.9), cv::Point(ipc.GetRows() * 1.3, ipc.GetRows() * 1.2), cv::Scalar(1), -1);
  // cv::Mat image1 = LoadUnitFloatImage<IPC::Float>(GetProjectPath("data/ipc/airplane.png"));
  cv::Mat crop1 = RoiCropMid(image1, ipc.mCols, ipc.mRows);
  crop1 += GetNoise<IPC::Float>(crop1.size(), noiseStdev);
  cv::Mat image2 = image1.clone();
  cv::Mat crop2;
  const auto iters = std::max(201., maxShift * 10 + 1);
  cv::Mat noise2 = GetNoise<IPC::Float>(crop1.size(), noiseStdev);
  std::vector<cv::Point2d> shifts;
  std::vector<cv::Point2d> ipcshifts;
  std::vector<cv::Point2d> pcshifts;

  // TODO: also test x only y only diag only

  // const cv::Point2d shiftOffset(25 + -maxShift * 0.5, -33 + -maxShift * 0.5);
  const cv::Point2d shiftOffset(-maxShift * 0.5, -maxShift * 0.5);
  for (int i = 0; i < iters; ++i)
  {
    ipc.SetDebugIndex(i);
    const auto shift = cv::Point2d(maxShift * i / (iters - 1), maxShift * i / (iters - 1)) + shiftOffset;
    const cv::Mat Tmat = (cv::Mat_<double>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
    cv::warpAffine(image1, image2, Tmat, image2.size());
    crop2 = RoiCropMid(image2, ipc.mCols, ipc.mRows) + noise2;
    ipc.SetDebugTrueShift(shift);
    Plot::PlotLeft("image1", crop1);
    Plot::PlotRight("image2", crop2);
    const auto ipcshift = ipc.Calculate(crop1, crop2);
    const auto pcshift = cv::phaseCorrelate(crop1, crop2);

    shifts.push_back(shift);
    ipcshifts.push_back(ipcshift);
    pcshifts.push_back(pcshift);
    LOG_DEBUG("Artificial shift = {} / Estimated shift = {} / IPC Error = {} / PC error = {}", shift, ipcshift, ipcshift - shift, pcshift - shift);
  }

  std::vector<cv::Point2d> ipcErrors(ipcshifts.size());
  std::vector<cv::Point2d> pcErrors(ipcshifts.size());
  std::ranges::transform(ipcshifts, shifts, ipcErrors.begin(), std::minus{});
  std::ranges::transform(pcshifts, shifts, pcErrors.begin(), std::minus{});

  std::vector<double> shiftsX(shifts.size());
  std::vector<double> shiftsY(shifts.size());
  std::ranges::transform(shifts, shiftsX.begin(), [](cv::Point2d a) { return a.x; });
  std::ranges::transform(shifts, shiftsY.begin(), [](cv::Point2d a) { return a.y; });

  std::vector<double> ipcShiftsX(ipcshifts.size());
  std::vector<double> pcShiftsX(ipcshifts.size());
  std::ranges::transform(ipcshifts, ipcShiftsX.begin(), [](cv::Point2d a) { return a.x; });
  std::ranges::transform(pcshifts, pcShiftsX.begin(), [](cv::Point2d a) { return a.x; });

  std::vector<double> ipcShiftsY(ipcshifts.size());
  std::vector<double> pcShiftsY(ipcshifts.size());
  std::ranges::transform(ipcshifts, ipcShiftsY.begin(), [](cv::Point2d a) { return a.y; });
  std::ranges::transform(pcshifts, pcShiftsY.begin(), [](cv::Point2d a) { return a.y; });

  std::vector<double> ipcErrorsAbs(ipcshifts.size());
  std::vector<double> pcErrorsAbs(ipcshifts.size());
  std::ranges::transform(ipcErrors, ipcErrorsAbs.begin(), [](cv::Point2d a) { return (std::abs(a.x) + std::abs(a.y)) / 2; });
  std::ranges::transform(pcErrors, pcErrorsAbs.begin(), [](cv::Point2d a) { return (std::abs(a.x) + std::abs(a.y)) / 2; });

  Plot::Plot({
      .name = "shifts x",
      .x = shiftsX,
      .ys = {shiftsX, pcShiftsX, ipcShiftsX},
      .ylabels = {"true x", "pc x", "ipc x"},
      .xlabel = "true shift x [px]",
      .ylabel = "calculated shift x [px]",
  });

  Plot::Plot({
      .name = "shifts y",
      .x = shiftsY,
      .ys = {shiftsY, pcShiftsY, ipcShiftsY},
      .ylabels = {"true y", "pc y", "ipc y"},
      .xlabel = "true shift y [px]",
      .ylabel = "calculated shift y [px]",
  });

  Plot::Plot({.name = "errors", .x = shiftsX, .ys = {pcErrorsAbs, ipcErrorsAbs}, .ylabels = {"pc error", "ipc error"}, .xlabel = "true shift [px]", .ylabel = "error [px]"});

  LOG_INFO("PC average error = {:.3f} ± {:.3f}", Mean(pcErrorsAbs), Stddev(pcErrorsAbs));
  LOG_INFO("IPC average error = {:.3f} ± {:.3f}", Mean(ipcErrorsAbs), Stddev(ipcErrorsAbs));
}

void IPCDebug::DebugUC(const IPC& ipc, double maxShift, double noiseStdev)
{
  LOG_FUNCTION;
  cv::Mat image1 = cv::Mat::zeros(ipc.GetRows() * 2, ipc.GetCols() * 2, CV_64F);
  cv::rectangle(image1, cv::Point(ipc.GetRows() * 1.1, ipc.GetRows() * 0.9), cv::Point(ipc.GetRows() * 1.3, ipc.GetRows() * 1.2), cv::Scalar(1), -1);
  cv::Mat crop1 = RoiCropMid(image1, ipc.mCols, ipc.mRows);
  crop1 += GetNoise<IPC::Float>(crop1.size(), noiseStdev);
  const int iters = std::max(101., maxShift * 10 + 1);
  cv::Mat noise2 = GetNoise<IPC::Float>(crop1.size(), noiseStdev);
  const cv::Point2d shiftOffset(-maxShift * 0.5, -maxShift * 0.5);
  const int L2Ustep = 2;
  const int L2Umin = 15;
  const int L2Usteps = 250;
  std::vector<double> L2Usizes(L2Usteps);
  std::vector<double> ipcErrorsAbs(L2Usteps);

#pragma omp parallel for
  for (int l = 0; l < L2Usteps; ++l)
  {
    const int L2Usize = L2Umin + l * L2Ustep;
    auto testIPC = ipc;
    testIPC.SetL2Usize(L2Usize);
    double error = 0;
    cv::Mat image2 = image1.clone();
    for (int i = 0; i < iters; ++i)
    {
      const auto shift = cv::Point2d(maxShift * i / (iters - 1), maxShift * i / (iters - 1)) + shiftOffset;
      const cv::Mat Tmat = (cv::Mat_<double>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::warpAffine(image1, image2, Tmat, image2.size());
      const cv::Mat crop2 = RoiCropMid(image2, testIPC.mCols, testIPC.mRows) + noise2;
      const auto ipcshift = testIPC.Calculate(crop1, crop2);
      error += 0.5 * std::abs(ipcshift.x - shift.x) + 0.5 * std::abs(ipcshift.y - shift.y);
    }
    L2Usizes[l] = L2Usize;
    ipcErrorsAbs[l] = error / iters;
    LOG_DEBUG("{}/{} Calculating L2Usize = {}: error = {:.3f}px", l + 1, L2Usteps, L2Usize, error / iters);
  }
  Plot::Plot({.name = "L2Usizes", .x = L2Usizes, .ys = {ipcErrorsAbs}, .ylabels = {"error"}, .xlabel = "L2Usize [px]", .ylabel = "error [px]"});
}
