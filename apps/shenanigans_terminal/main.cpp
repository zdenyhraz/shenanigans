#include "Random/ObjectDetection.hpp"

int main(int argc, char** argv)
try
{
  LOG_FUNCTION;
  auto image = LoadUnitFloatImage<f64>("../data/debug/shipwreck.jpg");
  cv::normalize(image, image, 0, 255, cv::NORM_MINMAX);
  image.convertTo(image, CV_8U);
  const auto objectSize = 50;

  {
    const auto objectThreshold = 0.15;
    const auto blurSize = 11;
    const auto stddevSize = 9;
    DetectObjectsStddev(image, objectSize, objectThreshold, blurSize, stddevSize);
  }

  {
    const auto objectThreshold = 0.03;
    const auto blurSize = 11;
    const auto sobelSize = 3;
    const auto lowThreshold = 0.3;
    const auto highThreshold = 3 * lowThreshold;
    DetectObjectsCanny(image, objectSize, objectThreshold, blurSize, sobelSize, lowThreshold, highThreshold);
  }

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
  return EXIT_FAILURE;
}
catch (...)
{
  LOG_UNKNOWN_EXCEPTION;
  return EXIT_FAILURE;
}
