#include "Windows/Shenanigans/WindowShenanigans.hpp"

void PythonInitialize()
{
  static py::scoped_interpreter guard{};
}

void QtInitialize()
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

int main(int argc, char** argv)
try
{
  QtInitialize();
  PythonInitialize();

  QApplication application(argc, argv);
  WindowShenanigans window;
  window.show();
  return application.exec();
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
