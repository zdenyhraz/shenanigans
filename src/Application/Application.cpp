#include "WindowShenanigans.h"

int main(int argc, char** argv)
{
  srand(time(0));

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // DPI support
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps); // HiDPI pixmaps

  QApplication a(argc, argv);
  WindowShenanigans w;
  w.show();
  return a.exec();
}
