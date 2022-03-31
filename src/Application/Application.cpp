#include "Windows/Shenanigans/WindowShenanigans.hpp"

void QtInitialize()
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

void PythonInitialize()
{
  static py::scoped_interpreter guard{};
}

int main(int argc, char** argv)
try
{
  QtInitialize();
  PythonInitialize();
  QApplication application(argc, argv);
  WindowShenanigans windowshen;
  windowshen.show();
  application.exec();
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
