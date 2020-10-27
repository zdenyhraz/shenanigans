#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"
#include "Gui/Windows/IPC/WindowIPC.h"
#include "Gui/Windows/Diffrot/WindowDiffrot.h"
#include "Gui/Windows/Features/WindowFeatures.h"
#include "Gui/Windows/FITS/WindowFITS.h"
#include "Procedural/procedural.h"
#include "Snake/game.h"
#include "DebugStuff/DebugStuff.h"

Zdeny_PhD_Shenanigans::Zdeny_PhD_Shenanigans(QWidget *parent) : QMainWindow(parent)
{
  // create global data shared within windows
  globals = std::make_unique<Globals>();

  // create windows
  mWindows["ipc"] = std::make_unique<WindowIPC>(this, globals.get());
  mWindows["diffrot"] = std::make_unique<WindowDiffrot>(this, globals.get());
  mWindows["features"] = std::make_unique<WindowFeatures>(this, globals.get());
  mWindows["fits"] = std::make_unique<WindowFITS>(this, globals.get());

  // setup Qt ui - meta compiled
  ui.setupUi(this);

  // show the logo
  QPixmap pm("Resources/logo.png");
  ui.label_2->setPixmap(pm);
  ui.label_2->setScaledContents(true);

  // init logger
  Logger::Init();
  LOG_SUCC("Welcome back, my friend.");

  // make signal - slot connections
  connect(ui.actionIPC, SIGNAL(triggered()), this, SLOT(ShowWindowIPC()));
  connect(ui.actionDiffrot, SIGNAL(triggered()), this, SLOT(ShowWindowDiffrot()));
  connect(ui.actionFeatures, SIGNAL(triggered()), this, SLOT(ShowWindowFeatures()));
  connect(ui.actionFITS, SIGNAL(triggered()), this, SLOT(ShowWindowFITS()));
  connect(ui.pushButtonDebug, SIGNAL(clicked()), this, SLOT(Debug()));
  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(About()));
  connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(CloseAll()));
  connect(ui.actionSnake, SIGNAL(triggered()), this, SLOT(Snake()));
  connect(ui.actionProcedural, SIGNAL(triggered()), this, SLOT(GenerateLand()));
}

void Zdeny_PhD_Shenanigans::ShowWindowIPC() { mWindows["ipc"]->show(); }

void Zdeny_PhD_Shenanigans::ShowWindowDiffrot() { mWindows["diffrot"]->show(); }

void Zdeny_PhD_Shenanigans::ShowWindowFeatures() { mWindows["features"]->show(); }

void Zdeny_PhD_Shenanigans::ShowWindowFITS() { mWindows["fits"]->show(); }

void Zdeny_PhD_Shenanigans::Exit() { QApplication::exit(); }

void Zdeny_PhD_Shenanigans::Debug() { Debug::Debug(globals.get()); }

void Zdeny_PhD_Shenanigans::About()
{
  QMessageBox msgBox;
  msgBox.setText("All these shenanigans were created during my PhD studies.\n\nHave fun,\nZdenek Hrazdira\n2018-2020");
  msgBox.exec();
}

void Zdeny_PhD_Shenanigans::CloseAll()
{
  // close all opencv windows
  destroyAllWindows();

  // close all qcustomplot windows
  Plot1D::CloseAll();
  Plot2D::CloseAll();

  // close all shenanigans windows
  for (auto &window : mWindows)
    window.second->close();

  LOG_INFO("All windows closed");
}

void Zdeny_PhD_Shenanigans::GenerateLand()
{
  LOG_INFO("Generating some land...");
  Mat mat = procedural(1000, 1000);
  showimg(colorlandscape(mat), "procedural nature");
  LOG_INFO("Finished generating some land. Do you like it?");
}

void Zdeny_PhD_Shenanigans::Snake()
{
  LOG_INFO("Started playing snake. (It is ok, everyone needs some rest.. :))");
  SnakeGame();
  LOG_INFO("Finished playing snake. Did you enjoy it? *wink*");
}

void Zdeny_PhD_Shenanigans::CloseEvent(QCloseEvent *event)
{
  QMessageBox::StandardButton resBtn = QMessageBox::question(
      this, "hehe XD", "Are you sure u wanna exit?\n", QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
  if (resBtn != QMessageBox::Yes)
  {
    event->ignore();
  }
  else
  {
    event->accept();
    CloseAll();
  }
}
