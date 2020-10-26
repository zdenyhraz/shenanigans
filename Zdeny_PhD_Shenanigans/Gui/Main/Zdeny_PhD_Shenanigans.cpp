#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"
#include "Optimization/Evolution.h"
#include "Optimization/OptimizationTestFunctions.h"
#include "Procedural/procedural.h"
#include "Snake/game.h"
#include "Fit/polyfit.h"
#include "Fit/nnfit.h"
#include "IPC/IPC.h"
#include "IPC/IterativePhaseCorrelation.h"
#include "DebugStuff/DebugStuff.h"

Zdeny_PhD_Shenanigans::Zdeny_PhD_Shenanigans(QWidget *parent) : QMainWindow(parent)
{
  // create global data shared within windows
  globals = std::make_unique<Globals>();

  // create windows
  windowIPC = std::make_unique<WindowIPC>(this, globals.get());
  windowDiffrot = std::make_unique<WindowDiffrot>(this, globals.get());
  windowFeatures = std::make_unique<WindowFeatures>(this, globals.get());

  // setup Qt ui - meta compiled
  ui.setupUi(this);

  // show the logo
  QPixmap pm("Resources/logo.png");
  ui.label_2->setPixmap(pm);
  ui.label_2->setScaledContents(true);

  // init logger
  Logger::Init();
  LOG_SUCC("Welcome back, my friend.");

  // make signal to slot connections
  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(CloseAll()));
  connect(ui.actionIPC, SIGNAL(triggered()), this, SLOT(showWindowIPC()));
  connect(ui.pushButtonDebug, SIGNAL(clicked()), this, SLOT(debug()));
  connect(ui.actionDiffrot, SIGNAL(triggered()), this, SLOT(showWindowDiffrot()));
  connect(ui.actionSnake, SIGNAL(triggered()), this, SLOT(playSnake()));
  connect(ui.actionProcedural, SIGNAL(triggered()), this, SLOT(generateLand()));
  connect(ui.actionFits_downloader, SIGNAL(triggered()), this, SLOT(fitsDownloader()));
  connect(ui.actionFits_checker, SIGNAL(triggered()), this, SLOT(fitsDownloadChecker()));
  connect(ui.actionFeatures, SIGNAL(triggered()), this, SLOT(featureMatch()));
}

void Zdeny_PhD_Shenanigans::exit() { QApplication::exit(); }

void Zdeny_PhD_Shenanigans::debug() { Debug::Debug(globals.get()); }

void Zdeny_PhD_Shenanigans::about()
{
  QMessageBox msgBox;
  msgBox.setText("All these shenanigans were created during my PhD studies.\n\nHave fun,\nZdenek Hrazdira\n2018-2020");
  msgBox.exec();
}

void Zdeny_PhD_Shenanigans::CloseAll()
{
  destroyAllWindows();
  Plot1D::CloseAll();
  Plot2D::CloseAll();
  LOG_INFO("All image & plot windows closed");
}

void Zdeny_PhD_Shenanigans::showWindowIPC() { windowIPC->show(); }

void Zdeny_PhD_Shenanigans::showWindowDiffrot() { windowDiffrot->show(); }

void Zdeny_PhD_Shenanigans::generateLand()
{
  LOG_INFO("Generating some land...");
  Mat mat = procedural(1000, 1000);
  showimg(colorlandscape(mat), "procedural nature");
  LOG_INFO("Finished generating some land. Do you like it?");
}

void Zdeny_PhD_Shenanigans::playSnake()
{
  LOG_INFO("Started playing snake. (It is ok, everyone needs some rest.. :))");
  SnakeGame();
  LOG_INFO("Finished playing snake. Did you enjoy it? *wink*");
}

void Zdeny_PhD_Shenanigans::fitsDownloader() { fitsDownloaderImpl(); }

void Zdeny_PhD_Shenanigans::fitsDownloadChecker() { fitsDownloadCheckerImpl(); }

void Zdeny_PhD_Shenanigans::closeEvent(QCloseEvent *event)
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

void Zdeny_PhD_Shenanigans::featureMatch() { windowFeatures->show(); }
