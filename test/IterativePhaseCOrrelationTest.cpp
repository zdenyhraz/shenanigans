#include "IPC/IterativePhaseCorrelation.h"

TEST(IterativePhaseCorrelationTest, ZeroShift)
{
  cv::Mat img1 = loadImage("../resources/Shapes/shape.png");
  cv::Mat img2 = img1.clone();

  IterativePhaseCorrelation ipc(img1);
  const auto shift = ipc.Calculate(img1, img2);
  EXPECT_EQ(shift, cv::Point2f(0, 0));
}

TEST(IterativePhaseCorrelationTest, NonZeroShift)
{
  cv::Mat img1 = loadImage("../resources/Shapes/shape.png");
  cv::Mat img2 = loadImage("../resources/Shapes/shapes.png");

  IterativePhaseCorrelation ipc(img1);
  const auto shift = ipc.Calculate(img1, img2);
  EXPECT_NE(shift, cv::Point2f(0, 0));
}