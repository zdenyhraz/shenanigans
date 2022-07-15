#include "ImageRegistration/IPC.hpp"

int main(int argc, char** argv)
try
{
  LOG_FUNCTION;

  const f64 noiseStddev = 0;

  IPC ipc(1024, 1024);
  const auto image1Path = "../data/articles/ipc/pics/align/304A_proc.png";
  const auto image2Path = "../data/articles/ipc/pics/align/171A_proc.png";
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
