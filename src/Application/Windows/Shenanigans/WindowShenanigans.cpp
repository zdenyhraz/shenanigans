#include "WindowShenanigans.hpp"
#include "../IPC/WindowIPC.hpp"
#include "../Diffrot/WindowDiffrot.hpp"
#include "../Features/WindowFeatures.hpp"
#include "../FITS/WindowFITS.hpp"
#include "../Filtering/WindowFiltering.hpp"
#include "Random/Procedural.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/OptimizationTestFunctions.hpp"
#include "ML/RegressionModel.hpp"
#include "ML/ImageSegmentationModel.hpp"

WindowShenanigans::WindowShenanigans(QWidget* parent) : QMainWindow(parent), mWindowData(std::make_unique<WindowData>())
{
  // create windows
  mWindows["ipc"] = std::make_unique<WindowIPC>(this, mWindowData.get());
  mWindows["diffrot"] = std::make_unique<WindowDiffrot>(this, mWindowData.get());
  mWindows["features"] = std::make_unique<WindowFeatures>(this, mWindowData.get());
  mWindows["fits"] = std::make_unique<WindowFITS>(this, mWindowData.get());
  mWindows["filtering"] = std::make_unique<WindowFiltering>(this, mWindowData.get());

  // setup Qt ui - meta compiled
  ui.setupUi(this);

  // show the logo
  QPixmap pm("../data/app/logo.png");
  ui.label_2->setPixmap(pm);
  ui.label_2->setScaledContents(true);

  // set logging params
  Logger::SetLogLevel(Logger::LogLevel::Function);
  QtLogger::SetTextBrowser(ui.textBrowser);

  LOG_SUCCESS("Welcome back, my friend.");

  // make signal - slot connections
  connect(ui.actionIPC, SIGNAL(triggered()), this, SLOT(ShowWindowIPC()));
  connect(ui.actionDiffrot, SIGNAL(triggered()), this, SLOT(ShowWindowDiffrot()));
  connect(ui.actionFeatures, SIGNAL(triggered()), this, SLOT(ShowWindowFeatures()));
  connect(ui.actionFITS, SIGNAL(triggered()), this, SLOT(ShowWindowFITS()));
  connect(ui.actionFiltering, SIGNAL(triggered()), this, SLOT(ShowWindowFiltering()));
  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(About()));
  connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(CloseAll()));
  connect(ui.actionSnake, SIGNAL(triggered()), this, SLOT(Snake()));
  connect(ui.actionProcedural, SIGNAL(triggered()), this, SLOT(GenerateLand()));
  connect(ui.actionUnitTests, SIGNAL(triggered()), this, SLOT(UnitTests()));
  connect(ui.pushButtonDebug, SIGNAL(clicked()), this, SLOT(RandomShit()));
}

void WindowShenanigans::ShowWindowIPC()
{
  mWindows["ipc"]->show();
}

void WindowShenanigans::ShowWindowDiffrot()
{
  mWindows["diffrot"]->show();
}

void WindowShenanigans::ShowWindowFeatures()
{
  mWindows["features"]->show();
}

void WindowShenanigans::ShowWindowFITS()
{
  mWindows["fits"]->show();
}

void WindowShenanigans::ShowWindowFiltering()
{
  mWindows["filtering"]->show();
}

void WindowShenanigans::Exit()
{
  QApplication::exit();
}

void WindowShenanigans::About()
{
  QMessageBox msgBox;
  msgBox.setText("All these shenanigans were created during my PhD studies.\n\nHave fun,\nZdenek Hrazdira");
  msgBox.exec();
}

void WindowShenanigans::CloseAll()
{
  cv::destroyAllWindows();
  Plot::CloseAll();

  for (auto& [windowname, window] : mWindows)
    window->close();

  LOG_INFO("All windows closed");
}

void WindowShenanigans::GenerateLand()
{
  LOG_FUNCTION("Generate land");
  cv::Mat mat = Procedural::procedural(1000, 1000);
  Showimg(Procedural::colorlandscape(mat), "procedural nature");
}

void WindowShenanigans::UnitTests()
{
}

void WindowShenanigans::Snake()
{
  LOG_FUNCTION("Play snake");
}

void WindowShenanigans::closeEvent(QCloseEvent* event)
{
  CloseAll();
  LOG_INFO("Good bye.");
}

void WindowShenanigans::RandomShit()
try
{
  LOG_FUNCTION("RandomShit");

  // Showimg(LoadUnitFloatImage<f64>("../debug/AIA/171A.png"), "xd");

  if (0) // ML
  {
    // RegressionModelTest();
    ImageSegmentationModelTest();
  }
  else // ipc
  {
    // IPCMeasure::MeasureAccuracyMap(mWindowData->mIPC, LoadUnitFloatImage<IPC::Float>("../debug/shapes/shapef.png"), 101);
    // IPCOptimization::Optimize(mWindowData->mIPC, "../debug/ipcopt", "../debug/ipcopt", 1.0, 0.01, 51, 0.2, 6);
    dynamic_cast<WindowIPC&>(*mWindows["ipc"]).ShowDebugStuff();
  }
}
catch (const std::exception& e)
{
  LOG_ERROR("RandomShit() error: {}", e.what());
}
catch (...)
{
  LOG_ERROR("RandomShit() error: {}", "Unknown error");
}
