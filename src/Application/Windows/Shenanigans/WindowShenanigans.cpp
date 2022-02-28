#include "WindowShenanigans.h"
#include "../IPC/WindowIPC.h"
#include "../Diffrot/WindowDiffrot.h"
#include "../Features/WindowFeatures.h"
#include "../FITS/WindowFITS.h"
#include "../Filtering/WindowFiltering.h"
#include "Random/Procedural.h"
#include "Optimization/OptimizationTestFunctions.h"

WindowShenanigans::WindowShenanigans(QWidget* parent) : QMainWindow(parent)
{
  // data shared within windows
  mWindowData = std::make_unique<WindowData>();

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

  // set the logging text browser
  QtLogger::SetTextBrowser(ui.textBrowser);
  QtLogger::SetLogLevel(QtLogger::LogLevel::Function);

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

  if (0) // optimization / metaoptimization
  {
    const i32 N = 2;
    const i32 runs = 20;
    const i32 maxFunEvals = 1000;
    const f32 optimalFitness = -std::numeric_limits<f64>::max();
    const bool meta = false;
    Evolution Evo(N);
    Evo.mNP = 5 * N;
    Evo.mMutStrat = Evolution::RAND1;
    Evo.mLB = Zerovect(N, -5.0);
    Evo.mUB = Zerovect(N, +5.0);
    Evo.mMaxFunEvals = maxFunEvals;
    Evo.mOptimalFitness = optimalFitness;
    Evo.SetName("debug");
    Evo.SetParameterNames({"x", "y"});
    Evo.SetConsoleOutput(true);
    Evo.SetPlotOutput(true);
    Evo.SetPlotObjectiveFunctionLandscape(true);
    Evo.SetPlotObjectiveFunctionLandscapeIterations(meta ? 51 : 301);
    Evo.SetSaveProgress(true);

    if (meta)
      Evo.MetaOptimize(OptimizationTestFunctions::Rosenbrock, Evolution::ObjectiveFunctionValue, runs, maxFunEvals, optimalFitness);
    else
      Evo.Optimize(OptimizationTestFunctions::Rosenbrock);
  }
  if (1) // ipc debug stuff
  {
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