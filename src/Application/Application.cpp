#include "Windows/Shenanigans/WindowShenanigans.h"

int main(int argc, char** argv)
try
{
  PROFILE_APP("Shenanigans");
  py::scoped_interpreter guard{};
  std::srand(std::time(nullptr));
  qputenv("QT_SCALE_FACTOR", "2.0");
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QApplication a(argc, argv);
  WindowShenanigans w;
  w.show();
  return a.exec();
}
catch (const std::exception& e)
{
  fmt::print("Error: {}\n", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  fmt::print("Error: {}\n", "An unknown error has occured");
  return EXIT_FAILURE;
}
