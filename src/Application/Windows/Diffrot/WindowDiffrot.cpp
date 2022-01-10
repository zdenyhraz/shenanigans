
#include "WindowDiffrot.h"
#include "Astrophysics/diffrot.h"
#include "Astrophysics/diffrotFileIO.h"
#include "Optimization/Evolution.h"

WindowDiffrot::WindowDiffrot(QWidget* parent, WindowData* windowData) : QMainWindow(parent), mWindowData(windowData)
{
  ui.setupUi(this);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(calculateDiffrot()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(showResults()));
  connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(showIPC()));
  connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(checkDiskShifts()));
  connect(ui.pushButton_6, SIGNAL(clicked()), this, SLOT(saveDiffrot()));
  connect(ui.pushButton_7, SIGNAL(clicked()), this, SLOT(loadDiffrot()));
  connect(ui.pushButton_8, SIGNAL(clicked()), this, SLOT(optimizeDiffrot()));
  connect(ui.pushButton_9, SIGNAL(clicked()), this, SLOT(video()));
  connect(ui.pushButton_10, SIGNAL(clicked()), this, SLOT(movingPeak()));
}

void WindowDiffrot::calculateDiffrot()
{
  LOG_FUNCTION("CalculateDiffrot");
  FitsTime time = GetStartFitsTime();
  drres = calculateDiffrotProfile(*mWindowData->IPC, time, drset);
  showResults();
  saveDiffrot();
}

void WindowDiffrot::showResults()
{
  drres.ShowResults(ui.lineEdit_20->text().toDouble(), ui.lineEdit_16->text().toDouble(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble());
  LOG_SUCCESS("Differential rotation profile shown.");
}

void WindowDiffrot::showIPC()
{
}

void WindowDiffrot::checkDiskShifts()
{
}

void WindowDiffrot::saveDiffrot()
{
  SaveDiffrotResultsToFile(ui.lineEdit_9->text().toStdString(), ui.lineEdit_23->text().toStdString(), &drres, mWindowData->IPC.get());
}

void WindowDiffrot::loadDiffrot()
{
  LoadDiffrotResultsFromFile(ui.lineEdit_24->text().toStdString(), &drres);
  drres.saveDir = ui.lineEdit_9->text().toStdString();
}

void WindowDiffrot::optimizeDiffrot()
{
  LOG_FUNCTION("OptimizeDiffrot");
  FitsTime starttime = GetStartFitsTime();

  try
  {
    auto f = [&](const std::vector<f64>& args)
    {
      i32 winsize = std::floor(args[5]);
      i32 L2size = std::floor(args[2]);
      i32 upsampleCoeff = std::floor(args[6]);
      winsize = winsize % 2 ? winsize + 1 : winsize;
      L2size = L2size % 2 ? L2size : L2size + 1;
      upsampleCoeff = upsampleCoeff % 2 ? upsampleCoeff : upsampleCoeff + 1;
      IterativePhaseCorrelation ipc_opt(winsize, winsize, args[0], args[1]);
      ipc_opt.SetL2size(L2size);
      ipc_opt.SetUpsampleCoeff(upsampleCoeff);
      // ipc_opt.SetApplyBandpass(args[3] > 0 ? true : false);
      // ipc_opt.SetApplyWindow(args[4] > 0 ? true : false);
      // ipc_opt.SetInterpolationType(args[7] > 0 ? cv::INTER_CUBIC : cv::INTER_LINEAR);
      FitsTime time_opt = starttime;
      DiffrotSettings drset_opt = drset;
      drset_opt.speak = false;
      return calculateDiffrotProfile(ipc_opt, time_opt, drset_opt).GetError();
    };

    const i32 runs = 2;
    for (i32 run = 0; run < runs; ++run)
    {
      Evolution evo(8);
      evo.mNP = 50;
      evo.mMutStrat = Evolution::RAND1;
      evo.mLB = {0.0001, 0.0001, 5, -1, -1, 64, 5, -1};
      evo.mUB = {5, 500, 17, 1, 1, 350, 51, 1};
      evo.SetFileOutputDir("E:\\WindowShenanigans\\articles\\diffrot\\temp\\");
      evo.SetParameterNames({"BPL", "BPH", "L2", "+BP", "+HANN", "WSIZE", "UC", "+BICUBIC"});
      evo.SetName(std::string("diffrot full") + " p" + std::to_string(drset.pics) + " s" + std::to_string(drset.sPic) + " y" + std::to_string(drset.ys));
      auto result = evo.Optimize(f).optimum;
      LOG_SUCCESS("Evolution run {}/{} result = {}", run + 1, runs, result);
    }
  }
  catch (...)
  {
    LOG_ERROR("Evolution optimization somehow failed");
  }
}

void WindowDiffrot::UpdateDrset()
{
  drset.pics = ui.lineEdit_7->text().toDouble();
  drset.ys = ui.lineEdit_2->text().toDouble();
  drset.sPic = ui.lineEdit_6->text().toDouble();
  drset.dPic = ui.lineEdit_5->text().toDouble();
  drset.vFov = ui.lineEdit_4->text().toDouble();
  drset.dSec = ui.lineEdit_8->text().toDouble();
  drset.medianFilter = ui.checkBox->isChecked();
  drset.movavgFilter = ui.checkBox_2->isChecked();
  drset.medianFilterSize = ui.lineEdit_21->text().toDouble();
  drset.movavgFilterSize = ui.lineEdit_22->text().toDouble();
  drset.visual = ui.checkBox_3->isChecked();
  drset.savepath = ui.lineEdit_9->text().toStdString();
  drset.sy = ui.lineEdit_25->text().toDouble();
  drset.pred = ui.checkBox_4->isChecked();
}

void WindowDiffrot::video()
{
  LOG_FUNCTION("Video");
  FitsTime time = GetStartFitsTime();

  drset.video = true;
  calculateDiffrotProfile(*mWindowData->IPC, time, drset);
  drset.video = false;
}

void WindowDiffrot::movingPeak()
{
}

FitsTime WindowDiffrot::GetStartFitsTime()
{
  UpdateDrset();
  return FitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(),
      ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
}
