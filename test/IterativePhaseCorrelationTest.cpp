#include "IPC/IterativePhaseCorrelation.hpp"

class IterativePhaseCorrelationTest : public ::testing::Test
{
protected:
  IterativePhaseCorrelationTest() : mImg1(LoadUnitFloatImage<f64>("../test/data/baboon.png")) {}

  void SetUp() override
  {
    cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., mShift.x, 0., 1., mShift.y);
    warpAffine(mImg1, mImg2, T, mImg2.size());
    ASSERT_TRUE(not mImg1.empty());
    ASSERT_TRUE(not mImg2.empty());
  }

  template <typename T = f64>
  IterativePhaseCorrelation<T> GetIPC() const
  {
    return IterativePhaseCorrelation<T>(mImg1.size());
  }

  const cv::Point2d mShift = cv::Point2d(38.638, -67.425);
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
  EXPECT_NEAR(shift.x, mShift.x, 0.3);
  EXPECT_NEAR(shift.y, mShift.y, 0.3);
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
  const auto subpixeliterativeError = ipc.Calculate<false, false, IterativePhaseCorrelation<f64>::AccuracyType::SubpixelIterative>(mImg1, mImg2) - mShift;
  const auto subpixelError = ipc.Calculate<false, false, IterativePhaseCorrelation<f64>::AccuracyType::Subpixel>(mImg1, mImg2) - mShift;
  const auto pixelError = ipc.Calculate<false, false, IterativePhaseCorrelation<f64>::AccuracyType::Pixel>(mImg1, mImg2) - mShift;
  EXPECT_LT(std::abs(subpixeliterativeError.x), std::abs(subpixelError.x));
  EXPECT_LT(std::abs(subpixeliterativeError.y), std::abs(subpixelError.y));
  EXPECT_LT(std::abs(subpixelError.x), std::abs(pixelError.x));
  EXPECT_LT(std::abs(subpixelError.y), std::abs(pixelError.y));
  EXPECT_LT(std::abs(pixelError.x), 0.5);
  EXPECT_LT(std::abs(pixelError.y), 0.5);
}

TEST_F(IterativePhaseCorrelationTest, FloatTypes)
{
  const auto ipc32 = GetIPC<f32>();
  const auto ipc64 = GetIPC<f64>();
  const auto shift32 = ipc32.Calculate(mImg1, mImg2);
  const auto shift64 = ipc64.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift32.x, shift64.x, kTolerance);
  EXPECT_NEAR(shift32.y, shift64.y, kTolerance);
}
