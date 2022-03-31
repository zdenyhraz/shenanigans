
void PythonInitialize()
{
  static py::scoped_interpreter guard{};
}

int main(int argc, char** argv)
try
{
  LOG_DEBUG("hi mom");
  PythonInitialize();
  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  fmt::print("Error: {}\n", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  fmt::print("Error: Unknown error\n");
  return EXIT_FAILURE;
}
