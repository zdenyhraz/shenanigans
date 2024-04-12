#include "NDA/Clarity/ClarityApp.hpp"

int main(int argc, char** argv)
try
{
  ClarityApp app;
  app.Run();
  std::this_thread::sleep_for(std::chrono::seconds(2));
  app.SimulateIncomingMessages();
  std::this_thread::sleep_for(std::chrono::seconds(3));
  app.Shutdown();
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
