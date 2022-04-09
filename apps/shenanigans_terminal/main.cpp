#include "ML/RegressionModel.hpp"

int main(int argc, char** argv)
try
{
  LOG_FUNCTION;
  RegressionModelTest();
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
