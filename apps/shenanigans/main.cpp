#include "Application.hpp"

int main(int argc, char** argv)
try
{
  Application app;
  app.Run();
  return EXIT_SUCCESS;
}
catch (const ShenanigansException& e)
{
  e.Log();
  return EXIT_FAILURE;
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
