#include "IPC/IterativePhaseCorrelation.h"

class IterativePhaseCorrelationTest : public ::testing::Test
{
protected:
  IterativePhaseCorrelationTest() : mImg1(loadImage("../test/data/shape.png")), mIPC(std::make_unique<IterativePhaseCorrelation>(mImg1.size())) {}

  void SetUp() override
  {
    cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., mShift.x, 0., 1., mShift.y);
    warpAffine(mImg1, mImg2, T, mImg2.size());
    ASSERT_TRUE(not mImg1.empty());
    ASSERT_TRUE(not mImg2.empty());
  }

  const cv::Point2d mShift = cv::Point2d(128.638, -67.425);
  static constexpr f64 mTolerance = 1e6;
  cv::Mat mImg1;
  cv::Mat mImg2;
  std::unique_ptr<IterativePhaseCorrelation> mIPC;
};

TEST_F(IterativePhaseCorrelationTest, BadInputs)
{
  EXPECT_THROW(mIPC->Calculate(mImg1, cv::Mat::ones(mImg1.rows + 1, mImg1.cols + 1, CV_32F)), std::runtime_error);
  EXPECT_THROW(mIPC->Calculate(mImg1, cv::Mat::ones(mImg1.rows, mImg1.cols, CV_32FC3)), std::runtime_error);
}

TEST_F(IterativePhaseCorrelationTest, Consistency)
{
  const auto shift1 = mIPC->Calculate(mImg1, mImg1);
  const auto shift2 = mIPC->Calculate(mImg1, mImg1);
  EXPECT_EQ(shift1, shift2);
}

TEST_F(IterativePhaseCorrelationTest, ZeroShift)
{
  const auto shift = mIPC->Calculate(mImg1, mImg1);
  EXPECT_NEAR(shift.x, 0, mTolerance);
  EXPECT_NEAR(shift.y, 0, mTolerance);
}

TEST_F(IterativePhaseCorrelationTest, Shift)
{
  const auto shift = mIPC->Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift.x, mShift.x, 0.3);
  EXPECT_NEAR(shift.y, mShift.y, 0.3);
}

TEST_F(IterativePhaseCorrelationTest, UnnormalizedInputs)
{
  const auto normShift = mIPC->Calculate(mImg1, mImg2);
  const auto unnormShift = mIPC->Calculate(mImg1 * 25.73, mImg2 * 38.14);
  EXPECT_NEAR(normShift.x, unnormShift.x, mTolerance);
  EXPECT_NEAR(normShift.y, unnormShift.y, mTolerance);
}

TEST_F(IterativePhaseCorrelationTest, AccuracyTypes)
{
  const auto subpixeliterativeError = mIPC->Calculate<false, false, IterativePhaseCorrelation::AccuracyType::SubpixelIterative>(mImg1, mImg2) - mShift;
  const auto subpixelError = mIPC->Calculate<false, false, IterativePhaseCorrelation::AccuracyType::Subpixel>(mImg1, mImg2) - mShift;
  const auto pixelError = mIPC->Calculate<false, false, IterativePhaseCorrelation::AccuracyType::Pixel>(mImg1, mImg2) - mShift;
  EXPECT_LT(std::abs(subpixeliterativeError.x), std::abs(subpixelError.x));
  EXPECT_LT(std::abs(subpixeliterativeError.y), std::abs(subpixelError.y));
  EXPECT_LT(std::abs(subpixelError.x), std::abs(pixelError.x));
  EXPECT_LT(std::abs(subpixelError.y), std::abs(pixelError.y));
  EXPECT_LT(std::abs(pixelError.x), 0.5);
  EXPECT_LT(std::abs(pixelError.y), 0.5);
}
