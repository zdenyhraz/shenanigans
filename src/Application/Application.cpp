#include "Windows/Shenanigans/WindowShenanigans.hpp"

void RandInitialize()
{
  std::srand(std::time(nullptr));
}

void PythonInitialize()
{
  static py::scoped_interpreter guard{};
}

void QtInitialize()
{
  qputenv("QT_SCALE_FACTOR", "2.0");
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

int main(int argc, char** argv)
try
{
  RandInitialize();
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
