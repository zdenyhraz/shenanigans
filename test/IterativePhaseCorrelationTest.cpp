#include "IPC/IterativePhaseCorrelation.h"

class IterativePhaseCorrelationTest : public ::testing::Test
{
protected:
  IterativePhaseCorrelationTest()
      : mImg1(loadImage("../resources/Shapes/shape.png")), mImg2(loadImage("../resources/Shapes/shapes.png")), mIPC(std::make_unique<IterativePhaseCorrelation>(mImg1.size()))
  {
  }

  const cv::Mat mImg1;
  const cv::Mat mImg2;
  const std::unique_ptr<IterativePhaseCorrelation> mIPC;
};

TEST_F(IterativePhaseCorrelationTest, ZeroShiftPhaseCorrel)
{
  const auto shift = mIPC->Calculate(mImg1, mImg1);
  EXPECT_EQ(shift, cv::Point2f(0, 0));
}

TEST_F(IterativePhaseCorrelationTest, ZeroShiftCrossCorrel)
{
  const auto shift = mIPC->Calculate<false, true>(mImg1, mImg1);
  EXPECT_EQ(shift, cv::Point2f(0, 0));
}

TEST_F(IterativePhaseCorrelationTest, ShiftPhaseCorrel)
{
  const auto shift = mIPC->Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift.x, 97.011337280273438, 1e-5);
  EXPECT_NEAR(shift.y, -103.99453735351562, 1e-5);
}

TEST_F(IterativePhaseCorrelationTest, ShiftCrossCorrel)
{
  const auto shift = mIPC->Calculate<false, true>(mImg1, mImg2);
  EXPECT_NEAR(shift.x, 97, 0.5);
  EXPECT_NEAR(shift.y, -104, 0.5);
}

TEST_F(IterativePhaseCorrelationTest, UnnormalizedInputsPhaseCorrel)
{
  const auto normShift = mIPC->Calculate(mImg1, mImg2);
  const auto unnormShift = mIPC->Calculate(mImg1 * 25.73, mImg2 * 38.14);
  EXPECT_EQ(normShift, unnormShift);
}

TEST_F(IterativePhaseCorrelationTest, UnnormalizedInputsCrossCorrel)
{
  const auto normShift = mIPC->Calculate<false, true>(mImg1, mImg2);
  const auto unnormShift = mIPC->Calculate<false, true>(mImg1 * 25.73, mImg2 * 38.14);
  EXPECT_EQ(normShift, unnormShift);
}