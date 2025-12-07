#include <gtest/gtest.h>
#include "ImageRegistration/IPC.hpp"
#include "Math/Transform.hpp"

class IPCTest : public ::testing::Test
{
protected:
  IPCTest() : mImg1(1000, 1000, CV_32F) { cv::randu(mImg1, cv::Scalar(0), cv::Scalar(1)); }

  using InterpolationType = IPC::InterpolationType;
  using BandpassType = IPC::BandpassType;
  using WindowType = IPC::WindowType;

  void SetUp() override
  {
    mImg2 = mImg1.clone();
    Shift(mImg2, mShift);
    ASSERT_TRUE(not mImg1.empty());
    ASSERT_TRUE(not mImg2.empty());
  }

  IPC GetIPC() const { return IPC(mImg1.size()); }

  cv::Point2d mShift = cv::Point2d(38.638, -67.425);
  // cppcheck-suppress unusedStructMember
  static constexpr double kTolerance = 1e-7;
  cv::Mat mImg1;
  cv::Mat mImg2;
};

TEST_F(IPCTest, ZeroShift)
{
  const auto ipc = GetIPC();
  const auto shift = ipc.Calculate(mImg1, mImg1);
  EXPECT_NEAR(shift.x, 0, kTolerance);
  EXPECT_NEAR(shift.y, 0, kTolerance);
}

TEST_F(IPCTest, BasicShift)
{
  const auto ipc = GetIPC();
  const auto shift = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift.x, mShift.x, 0.5);
  EXPECT_NEAR(shift.y, mShift.y, 0.5);
}

TEST_F(IPCTest, GradualShift)
{
  IPC ipc(512, 512);
  ipc.SetBandpassType(IPC::BandpassType::None);
  const auto w = ipc.GetCols();
  const auto h = ipc.GetRows();
  ASSERT_LT(w, mImg1.cols);
  ASSERT_LT(h, mImg1.rows);
  const cv::Point2d maxShift(1, 0);
  const auto tolerance = 0.1;
  const auto iters = 51;
  for (int i = 0; i < iters; ++i)
  {
    const auto targetShift = static_cast<double>(i) / (iters - 1) * maxShift;
    const cv::Mat Tmat = (cv::Mat_<double>(2, 3) << 1., 0., targetShift.x, 0., 1., targetShift.y);
    cv::Mat imageShifted;
    cv::warpAffine(mImg1, imageShifted, Tmat, mImg1.size());
    const cv::Mat image1 = RoiCropMid(mImg1, w, h);
    const cv::Mat image2 = RoiCropMid(imageShifted, w, h);
    const auto ipcShift = ipc.Calculate(image1, image2);
    const auto pcShift = cv::phaseCorrelate(image1, image2);
    const auto ipcShiftError = cv::Point2d(std::abs(ipcShift.x - targetShift.x), std::abs(ipcShift.y - targetShift.y));
    const auto pcShiftError = cv::Point2d(std::abs(pcShift.x - targetShift.x), std::abs(pcShift.y - targetShift.y));
    // EXPECT_LE(ipcShiftError.x, pcShiftError.x);
    // EXPECT_LE(ipcShiftError.y, pcShiftError.y);
    EXPECT_NEAR(ipcShift.x, targetShift.x, tolerance);
    EXPECT_NEAR(ipcShift.y, targetShift.y, tolerance);
  }
}

TEST_F(IPCTest, LargeShift)
{
  const auto ipc = GetIPC();
  mImg2 = mImg1.clone();
  mShift = cv::Point2d(mImg2.cols / 2 * 0.995, 0.);
  Shift(mImg2, mShift);
  const auto shift = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift.x, mShift.x, 0.5);
  EXPECT_NEAR(shift.y, mShift.y, 0.5);
}

TEST_F(IPCTest, BadInputs)
{
  const auto ipc = GetIPC();
  EXPECT_THROW(ipc.Calculate(mImg1, cv::Mat::ones(mImg1.rows + 1, mImg1.cols + 1, CV_32F)), std::invalid_argument);
  EXPECT_THROW(ipc.Calculate(mImg1, cv::Mat::ones(mImg1.rows, mImg1.cols, CV_32FC3)), std::invalid_argument);
}

TEST_F(IPCTest, Consistency)
{
  const auto ipc = GetIPC();
  const auto shift1 = ipc.Calculate(mImg1, mImg1);
  const auto shift2 = ipc.Calculate(mImg1, mImg1);
  EXPECT_EQ(shift1, shift2);
}

TEST_F(IPCTest, UnnormalizedInputs)
{
  const auto ipc = GetIPC();
  const auto normShift = ipc.Calculate(mImg1, mImg2);
  const auto unnormShift = ipc.Calculate(mImg1 * 25.73, mImg2 * 38.14);
  EXPECT_NEAR(normShift.x, unnormShift.x, kTolerance);
  EXPECT_NEAR(normShift.y, unnormShift.y, kTolerance);
}

TEST_F(IPCTest, InterpolationTypes)
{
  auto ipc = GetIPC();

  ipc.SetInterpolationType(InterpolationType::NearestNeighbor);
  const auto shiftNearest = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftNearest.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftNearest.y, mShift.y, 0.5);

  ipc.SetInterpolationType(InterpolationType::Linear);
  const auto shiftLinear = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftLinear.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftLinear.y, mShift.y, 0.5);

  ipc.SetInterpolationType(InterpolationType::Cubic);
  const auto shiftCubic = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftCubic.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftCubic.y, mShift.y, 0.5);
}

TEST_F(IPCTest, BandpassTypes)
{
  auto ipc = GetIPC();
  ipc.SetBandpassParameters(0.1, 0.9);

  ipc.SetBandpassType(BandpassType::Rectangular);
  const auto shiftRectangular = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftRectangular.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftRectangular.y, mShift.y, 0.5);

  ipc.SetBandpassType(BandpassType::Gaussian);
  const auto shiftGaussian = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftGaussian.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftGaussian.y, mShift.y, 0.5);

  ipc.SetBandpassType(BandpassType::None);
  const auto shiftNone = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftNone.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftNone.y, mShift.y, 0.5);
}

TEST_F(IPCTest, WindowTypes)
{
  auto ipc = GetIPC();

  ipc.SetWindowType(WindowType::None);
  const auto shiftNone = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftNone.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftNone.y, mShift.y, 0.5);

  ipc.SetWindowType(WindowType::Hann);
  const auto shiftHann = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shiftHann.x, mShift.x, 0.5);
  EXPECT_NEAR(shiftHann.y, mShift.y, 0.5);
}
