#include "Gui/Shenanigans.hpp"

int main(int argc, char** argv)
try
{
  LOG_FUNCTION;
  Shenanigans::Run();
  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
  return EXIT_FAILURE;
}
catch (...)
{
  LOG_ERROR("Error: Unknown error");
  return EXIT_FAILURE;
}
