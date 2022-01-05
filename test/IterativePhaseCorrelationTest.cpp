#include "IPC/IterativePhaseCorrelation.h"

class IterativePhaseCorrelationTest : public ::testing::Test
{
protected:
  IterativePhaseCorrelationTest() : mImg1(loadImage("../resources/Shapes/shape.png")), mImg2(loadImage("../resources/Shapes/shapes.png")) {}

  const cv::Mat mImg1;
  const cv::Mat mImg2;
};

TEST_F(IterativePhaseCorrelationTest, ZeroShiftPhaseCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate(mImg1, mImg1);
  EXPECT_EQ(shift, cv::Point2f(0, 0));
}

TEST_F(IterativePhaseCorrelationTest, ZeroShiftCrossCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate<false, true>(mImg1, mImg1);
  EXPECT_EQ(shift, cv::Point2f(0, 0));
}

TEST_F(IterativePhaseCorrelationTest, ShiftPhaseCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate(mImg1, mImg2);
  EXPECT_NEAR(shift.x, 97, 0.5);
  EXPECT_NEAR(shift.y, -104, 0.5);
}

TEST_F(IterativePhaseCorrelationTest, ShiftCrossCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate<false, true>(mImg1, mImg2);
  EXPECT_NEAR(shift.x, 97, 0.5);
  EXPECT_NEAR(shift.y, -104, 0.5);
}

TEST_F(IterativePhaseCorrelationTest, UnnormalizedInputsPhaseCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto normShift = ipc.Calculate(mImg1, mImg2);
  const auto unnormShift = ipc.Calculate(mImg1 * 25.73, mImg2 * 38.14);
  EXPECT_EQ(normShift, unnormShift);
}

TEST_F(IterativePhaseCorrelationTest, UnnormalizedInputsCrossCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto normShift = ipc.Calculate<false, true>(mImg1, mImg2);
  const auto unnormShift = ipc.Calculate<false, true>(mImg1 * 25.73, mImg2 * 38.14);
  EXPECT_EQ(normShift, unnormShift);
}