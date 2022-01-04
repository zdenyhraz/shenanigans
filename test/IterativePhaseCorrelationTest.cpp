#include "IPC/IterativePhaseCorrelation.h"

class IterativePhaseCorrelationTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    mImg1 = loadImage("../resources/Shapes/shape.png");
    mImg2 = loadImage("../resources/Shapes/shapes.png");
  }

  cv::Mat mImg1;
  cv::Mat mImg2;
};

TEST_F(IterativePhaseCorrelationTest, ZeroShift)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate(mImg1, mImg1);
  EXPECT_EQ(shift, cv::Point2f(0, 0));
}

TEST_F(IterativePhaseCorrelationTest, NonZeroShift)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate(mImg1, mImg2);
  EXPECT_NE(shift, cv::Point2f(0, 0));
}

TEST_F(IterativePhaseCorrelationTest, ZeroShiftCrossCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate<false, true>(mImg1, mImg1);
  EXPECT_EQ(shift, cv::Point2f(0, 0));
}

TEST_F(IterativePhaseCorrelationTest, NonZeroShiftCrossCorrel)
{
  IterativePhaseCorrelation ipc(mImg1);
  const auto shift = ipc.Calculate<false, true>(mImg1, mImg2);
  EXPECT_NE(shift, cv::Point2f(0, 0));
}
