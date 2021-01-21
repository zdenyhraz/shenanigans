#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"
#include "Gui/Windows/IPC/WindowIPC.h"
#include "Gui/Windows/Diffrot/WindowDiffrot.h"
#include "Gui/Windows/Features/WindowFeatures.h"
#include "Gui/Windows/FITS/WindowFITS.h"
#include "Gui/Windows/Filtering/WindowFiltering.h"
#include "Procedural/procedural.h"
#include "Snake/game.h"
#include "DebugStuff/DebugStuff.h"
#include "UnitTests/UnitTests.h"

Zdeny_PhD_Shenanigans::Zdeny_PhD_Shenanigans(QWidget* parent) : QMainWindow(parent)
{
  // create global data shared within windows
  globals = std::make_unique<Globals>();

  // create windows
  mWindows["ipc"] = std::make_unique<WindowIPC>(this, globals.get());
  mWindows["diffrot"] = std::make_unique<WindowDiffrot>(this, globals.get());
  mWindows["features"] = std::make_unique<WindowFeatures>(this, globals.get());
  mWindows["fits"] = std::make_unique<WindowFITS>(this, globals.get());
  mWindows["filtering"] = std::make_unique<WindowFiltering>(this, globals.get());

  // setup Qt ui - meta compiled
  ui.setupUi(this);

  // show the logo
  QPixmap pm("Resources/logo.png");
  ui.label_2->setPixmap(pm);
  ui.label_2->setScaledContents(true);

  // set the logging text browser
  QtLogger::SetTextBrowser(ui.textBrowser);
  QtLogger::SetLogLevel(QtLogger::LogLevel::Trace);

  LOG_INFO("Welcome back, my friend.");

  // make signal - slot connections
  connect(ui.actionIPC, SIGNAL(triggered()), this, SLOT(ShowWindowIPC()));
  connect(ui.actionDiffrot, SIGNAL(triggered()), this, SLOT(ShowWindowDiffrot()));
  connect(ui.actionFeatures, SIGNAL(triggered()), this, SLOT(ShowWindowFeatures()));
  connect(ui.actionFITS, SIGNAL(triggered()), this, SLOT(ShowWindowFITS()));
  connect(ui.actionFiltering, SIGNAL(triggered()), this, SLOT(ShowWindowFiltering()));
  connect(ui.pushButtonDebug, SIGNAL(clicked()), this, SLOT(Debug()));
  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(About()));
  connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(CloseAll()));
  connect(ui.actionSnake, SIGNAL(triggered()), this, SLOT(Snake()));
  connect(ui.actionProcedural, SIGNAL(triggered()), this, SLOT(GenerateLand()));
  connect(ui.actionUnitTests, SIGNAL(triggered()), this, SLOT(UnitTests()));
}

void Zdeny_PhD_Shenanigans::ShowWindowIPC()
{
  mWindows["ipc"]->show();
}

void Zdeny_PhD_Shenanigans::ShowWindowDiffrot()
{
  mWindows["diffrot"]->show();
}

void Zdeny_PhD_Shenanigans::ShowWindowFeatures()
{
  mWindows["features"]->show();
}

void Zdeny_PhD_Shenanigans::ShowWindowFITS()
{
  mWindows["fits"]->show();
}

void Zdeny_PhD_Shenanigans::ShowWindowFiltering()
{
  mWindows["filtering"]->show();
}

void Zdeny_PhD_Shenanigans::Exit()
{
  QApplication::exit();
}

void Zdeny_PhD_Shenanigans::Debug()
{
  Debug::Debug(globals.get());
}

void Zdeny_PhD_Shenanigans::About()
{
  QMessageBox msgBox;
  msgBox.setText("All these shenanigans were created during my PhD studies.\n\nHave fun,\n© Zdenek Hrazdira");
  msgBox.exec();
}

void Zdeny_PhD_Shenanigans::CloseAll()
{
  destroyAllWindows();
  Plot::CloseAll();

  for (auto& [windowname, window] : mWindows)
    window->close();

  LOG_INFO("All windows closed");
}

void Zdeny_PhD_Shenanigans::GenerateLand()
{
  LOG_FUNCTION("Generate land");
  Mat mat = procedural(1000, 1000);
  showimg(colorlandscape(mat), "procedural nature");
}

void Zdeny_PhD_Shenanigans::UnitTests()
{
  UnitTests::TestAll();
}

void Zdeny_PhD_Shenanigans::Snake()
{
  LOG_FUNCTION("Play snake");
  SnakeGame();
}

void Zdeny_PhD_Shenanigans::closeEvent(QCloseEvent* event)
{
  QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Quit", "Are you sure you want to quit?    ", QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes);
  if (resBtn != QMessageBox::Yes)
  {
    event->ignore();
  }
  else
  {
    event->accept();
    CloseAll();
    LOG_SUCCESS("Good bye.");
  }
}
