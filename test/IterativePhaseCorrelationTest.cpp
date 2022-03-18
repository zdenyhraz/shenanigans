#include "IPC/IterativePhaseCorrelation.hpp"

class IterativePhaseCorrelationTest : public ::testing::Test
{
protected:
  IterativePhaseCorrelationTest() : mImg1(LoadUnitFloatImage<f64>("../test/data/baboon.png")) {}

  using ModeType = IterativePhaseCorrelation::ModeType;
  using CorrelationType = IterativePhaseCorrelation::CorrelationType;
  using InterpolationType = IterativePhaseCorrelation::InterpolationType;
  using BandpassType = IterativePhaseCorrelation::BandpassType;
  using WindowType = IterativePhaseCorrelation::WindowType;
  using AccuracyType = IterativePhaseCorrelation::AccuracyType;

  void SetUp() override
  {
    mImg2 = mImg1.clone();
    Shift(mImg2, mShift);
    ASSERT_TRUE(not mImg1.empty());
    ASSERT_TRUE(not mImg2.empty());
  }

  IterativePhaseCorrelation GetIPC() const { return IterativePhaseCorrelation(mImg1.size()); }

  cv::Point2d mShift = cv::Point2d(38.638, -67.425);
  static constexpr f64 kTolerance = 1e-7;
  cv::Mat mImg1;
  cv::Mat mImg2;
};

TEST_F(IterativePhaseCorrelationTest, BadInputs)
{
  const auto ipc = GetIPC();
  EXPECT_THROW(ipc.Calculate(mImg1, cv::Mat::ones(mImg1.rows + 1, mImg1.cols + 1, CV_32F)), std::invalid_argument);
  EXPECT_THROW(ipc.Calculate(mImg1, cv::Mat::ones(mImg1.rows, mImg1.cols, CV_32FC3)), std::invalid_argument);
}

TEST_F(IterativePhaseCorrelationTest, Consistency)
{
  const auto ipc = GetIPC();
  const auto shift1 = ipc.Calculate(mImg1, mImg1);
  const auto shift2 = ipc.Calculate(mImg1, mImg1);
  EXPECT_EQ(shift1, shift2);
}

TEST_F(IterativePhaseCorrelationTest, ZeroShift)
{
  const auto ipc = GetIPC();
  const auto shift = ipc.Calculate(mImg1, mImg1);
  EXPECT_NEAR(shift.x, 0, kTolerance);
  EXPECT_NEAR(shift.y, 0, kTolerance);
}

TEST_F(IterativePhaseCorrelationTest, Shift)
{
  const auto ipc = GetIPC();
  const auto shift = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift.x, mShift.x, 0.5);
  EXPECT_NEAR(shift.y, mShift.y, 0.5);
}

TEST_F(IterativePhaseCorrelationTest, LargeShift)
{
  const auto ipc = GetIPC();
  mImg2 = mImg1.clone();
  mShift = cv::Point2d(mImg2.cols / 2 * 0.995, 0.);
  Shift(mImg2, mShift);
  const auto shift = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift.x, mShift.x, 0.5);
  EXPECT_NEAR(shift.y, mShift.y, 0.5);
}

TEST_F(IterativePhaseCorrelationTest, UnnormalizedInputs)
{
  const auto ipc = GetIPC();
  const auto normShift = ipc.Calculate(mImg1, mImg2);
  const auto unnormShift = ipc.Calculate(mImg1 * 25.73, mImg2 * 38.14);
  EXPECT_NEAR(normShift.x, unnormShift.x, kTolerance);
  EXPECT_NEAR(normShift.y, unnormShift.y, kTolerance);
}

TEST_F(IterativePhaseCorrelationTest, AccuracyTypes)
{
  const auto ipc = GetIPC();
  const auto subpixeliterativeError = ipc.Calculate<{.AccuracyT = AccuracyType::SubpixelIterative}>(mImg1, mImg2) - mShift;
  const auto subpixelError = ipc.Calculate<{.AccuracyT = AccuracyType::Subpixel}>(mImg1, mImg2) - mShift;
  const auto pixelError = ipc.Calculate<{.AccuracyT = AccuracyType::Pixel}>(mImg1, mImg2) - mShift;
  EXPECT_LT(std::abs(subpixeliterativeError.x), std::abs(subpixelError.x));
  EXPECT_LT(std::abs(subpixeliterativeError.y), std::abs(subpixelError.y));
  EXPECT_LT(std::abs(subpixelError.x), std::abs(pixelError.x));
  EXPECT_LT(std::abs(subpixelError.y), std::abs(pixelError.y));
  EXPECT_LT(std::abs(pixelError.x), 0.5);
  EXPECT_LT(std::abs(pixelError.y), 0.5);
}

TEST_F(IterativePhaseCorrelationTest, CorrelationTypes)
{
  const auto ipc = GetIPC();
  EXPECT_NO_THROW((ipc.Calculate<{.CorrelationT = CorrelationType::PhaseCorrelation}>(mImg1, mImg2)));
  EXPECT_NO_THROW((ipc.Calculate<{.CorrelationT = CorrelationType::CrossCorrelation}>(mImg1, mImg2)));
}

TEST_F(IterativePhaseCorrelationTest, InterpolationTypes)
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

TEST_F(IterativePhaseCorrelationTest, BandpassTypes)
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

TEST_F(IterativePhaseCorrelationTest, WindowTypes)
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
