#include "WindowShenanigans.h"

i32 main(i32 argc, char** argv)
try
{
  srand(time(0));
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
}
catch (...)
{
  fmt::print("Error: {}\n", "An unknown error has occured");
}
