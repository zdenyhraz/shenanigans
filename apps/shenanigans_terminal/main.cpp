int main(int argc, char** argv)
try
{
  LOG_TERMINAL_DEBUG("hi mom");
  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  LOG_TERMINAL_ERROR("Error: {}", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  LOG_TERMINAL_ERROR("Error: Unknown error");
  return EXIT_FAILURE;
}
