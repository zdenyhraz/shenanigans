#include "Shenanigans.hpp"

int main(int argc, char** argv)
try
{
  Shenanigans::Run();
  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  TermLogger::Error("Error: {}", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  TermLogger::Error("Error: Unknown error");
  return EXIT_FAILURE;
}
