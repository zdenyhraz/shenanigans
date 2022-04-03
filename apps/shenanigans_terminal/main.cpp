int main(int argc, char** argv)
try
{
  TermLogger::Debug("hi mom");
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
