#include "ImageRegistration/IPC.hpp"

int main(int argc, char** argv)
try
{
  LOG_FUNCTION;

  PyPlot::SetSave(true);

  const auto image1Path = "/home/hrazdira/shenanigans/debug/tfs_1A.tif";
  const auto image2Path = "/home/hrazdira/shenanigans/debug/tfs_1B.tif";
  const f64 noiseStddev = 0;
  const auto image = LoadUnitFloatImage<IPC::Float>(image1Path);
  const auto size = std::min(image.rows, image.cols);
  const auto ipc = IPC(size, size, 0, 0.5);
  IPCDebug::DebugAlign(ipc, image1Path, image2Path, noiseStddev);

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
