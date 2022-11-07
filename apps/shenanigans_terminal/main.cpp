#include "Random/ObjectDetection.hpp"

int main(int argc, char** argv)
try
{
  LOG_FUNCTION;

  PyPlot::Initialize();
  PyPlot::SetSave(true);

  const auto imagePath = "../data/debug/shipwreck1.jpg";
  const auto image = LoadUnitFloatImage<f64>(imagePath);
  const auto size = 10;

  DetectObjects(image, size);

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
