#include "Random/ObjectDetection.hpp"

int main(int argc, char** argv)
try
{
  LOG_FUNCTION;

  const auto imagePath = "../data/debug/shipwreck1.jpg";
  const auto image = LoadUnitFloatImage<f64>(imagePath);
  const auto blurSize = 21;
  const auto stddevSize = 11;

  DetectObjects(image, blurSize, stddevSize);

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
