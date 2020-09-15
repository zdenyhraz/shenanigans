#include "stdafx.h"
#include "WindowDiffrot.h"
#include "Astrophysics/diffrot.h"
#include "Astrophysics/diffrotFileIO.h"
#include "Optimization/optimization.h"

WindowDiffrot::WindowDiffrot(QWidget *parent, Globals *globals) : QMainWindow(parent), globals(globals)
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
  LOG_STARTEND("Calculating diffrot profile...", "Differential rotation profile calculated.");
  FitsTime time = GetStartFitsTime();
  drres = calculateDiffrotProfile(*globals->IPCset, time, drset);
  showResults();
}

void WindowDiffrot::showResults()
{
  drres.ShowResults(ui.lineEdit_20->text().toDouble(), ui.lineEdit_16->text().toDouble(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble());
  LOG_SUCC("Differential rotation profile shown.");
}

void WindowDiffrot::showIPC()
{
  LOG_INFO("Showimg diffrot profile IPC landscape...");
  FitsParams params1, params2;
  IPCsettings set = *globals->IPCset;
  set.speak = IPCsettings::All;
  // set.save = true;
  FitsTime time = GetStartFitsTime();

  LOG_INFO("Loading file '" + time.path() + "'...");
  Mat pic1 = roicrop(loadfits(time.path(), params1), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows());

  time.advanceTime(ui.lineEdit_5->text().toDouble() * ui.lineEdit_8->text().toDouble());
  int predShift = 0;
  if (ui.checkBox_4->isChecked())
  {
    predShift = predictDiffrotShift(ui.lineEdit_5->text().toDouble(), ui.lineEdit_8->text().toDouble(), params1.R);
    LOG_DEBUG("Predicted shift used = {}", predShift);
  }

  LOG_INFO("Loading file '" + time.path() + "'...");
  Mat pic2 = roicrop(loadfits(time.path(), params1), params1.fitsMidX + predShift, params1.fitsMidY, set.getcols(), set.getrows());

  phasecorrel(pic1, pic2, set);
}

void WindowDiffrot::checkDiskShifts()
{
  LOG_INFO("Checking diffrot disk shifts...");
  FitsTime time = GetStartFitsTime();
  IPCsettings set = *globals->IPCset;

  int edgeN = 0.025 * 4096;
  int edgeS = 0.974 * 4096;
  int edgeW = 0.027 * 4096;
  int edgeE = 0.975 * 4096;
  int center = 0.5 * 4096;

  int pics = ui.lineEdit_7->text().toDouble();
  std::vector<double> shiftsN;
  std::vector<double> shiftsS;
  std::vector<double> shiftsW;
  std::vector<double> shiftsE;
  std::vector<double> shiftsFX;
  std::vector<double> shiftsFY;
  FitsImage pic1, pic2;
  int lag1, lag2;
  Mat picshow;

  for (int pic = 0; pic < pics; pic++)
  {
    LOG_DEBUG("{} / {} ...", pic + 1, pics);
    time.advanceTime((bool)pic * (ui.lineEdit_6->text().toDouble() - ui.lineEdit_5->text().toDouble()) * ui.lineEdit_8->text().toDouble());
    loadFitsFuzzy(pic1, time, lag1);
    time.advanceTime(ui.lineEdit_5->text().toDouble() * ui.lineEdit_8->text().toDouble());
    loadFitsFuzzy(pic2, time, lag2);

    if (!pic)
      picshow = pic1.image().clone();

    if (pic1.params().succload && pic2.params().succload)
    {
      shiftsN.push_back(phasecorrel(roicrop(pic1.image(), center, edgeN, set.getcols(), set.getrows()), roicrop(pic2.image(), center, edgeN, set.getcols(), set.getrows()), set).y);
      shiftsS.push_back(phasecorrel(roicrop(pic1.image(), center, edgeS, set.getcols(), set.getrows()), roicrop(pic2.image(), center, edgeS, set.getcols(), set.getrows()), set).y);
      shiftsW.push_back(phasecorrel(roicrop(pic1.image(), edgeW, center, set.getcols(), set.getrows()), roicrop(pic2.image(), edgeW, center, set.getcols(), set.getrows()), set).x);
      shiftsE.push_back(phasecorrel(roicrop(pic1.image(), edgeE, center, set.getcols(), set.getrows()), roicrop(pic2.image(), edgeE, center, set.getcols(), set.getrows()), set).x);
      shiftsFX.push_back(pic2.params().fitsMidX - pic1.params().fitsMidX);
      shiftsFY.push_back(pic2.params().fitsMidY - pic1.params().fitsMidY);
    }
  }

  LOG_INFO("<<<<<<<<<<<<<<<<<<   ABSIPC median   /   ABSFITS median   /   ABSDIFF median   >>>>>>>>>>>>>>>>>>>>>");
  LOG_INFO("Diffrot shifts N = {} / {} / {}", median(abs(shiftsN)), median(abs(shiftsFY)), median(abs(shiftsN - shiftsFY)));
  LOG_INFO("Diffrot shifts S = {} / {} / {}", median(abs(shiftsS)), median(abs(shiftsFY)), median(abs(shiftsS - shiftsFY)));
  LOG_INFO("Diffrot shifts W = {} / {} / {}", median(abs(shiftsW)), median(abs(shiftsFX)), median(abs(shiftsW - shiftsFX)));
  LOG_INFO("Diffrot shifts E = {} / {} / {}", median(abs(shiftsE)), median(abs(shiftsFX)), median(abs(shiftsE - shiftsFX)));

  std::vector<Mat> picsshow(4);
  picsshow[0] = roicrop(picshow, center, edgeN, set.getcols(), set.getrows());
  picsshow[3] = roicrop(picshow, center, edgeS, set.getcols(), set.getrows());
  picsshow[2] = roicrop(picshow, edgeW, center, set.getcols(), set.getrows());
  picsshow[1] = roicrop(picshow, edgeE, center, set.getcols(), set.getrows());
  showimg(picsshow, "pics");

  std::vector<double> iotam(shiftsFX.size());
  std::iota(iotam.begin(), iotam.end(), 0);
  iotam = (double)(ui.lineEdit_7->text().toDouble() - 1) * ui.lineEdit_6->text().toDouble() * 45 / 60 / 60 / 24 / (iotam.size() - 1) * iotam;
  Plot1D::plot(iotam, std::vector<std::vector<double>>{shiftsFX, shiftsW, shiftsE}, "shiftsX", "time [days]", "45sec shiftX [px]", std::vector<std::string>{"shifts fits header X", "shifts IPC west edge", "shifts IPC east edge"});
  Plot1D::plot(iotam, std::vector<std::vector<double>>{shiftsFY, shiftsN, shiftsS}, "shiftsY", "time [days]", "45sec shiftY [px]", std::vector<std::string>{"shifts fits header Y", "shifts IPC north edge", "shifts IPC south edge"});
}

void WindowDiffrot::saveDiffrot() { SaveDiffrotResultsToFile(ui.lineEdit_9->text().toStdString(), ui.lineEdit_23->text().toStdString(), &drres); }

void WindowDiffrot::loadDiffrot()
{
  LoadDiffrotResultsFromFile(ui.lineEdit_24->text().toStdString(), &drres);
  drres.saveDir = ui.lineEdit_9->text().toStdString();
}

void WindowDiffrot::optimizeDiffrot()
{
  LOG_STARTEND("Optimizing diffrot...", "Diffrot optimized");
  FitsTime time = GetStartFitsTime();

  if (1) // all variables
  {
    auto f = [&](const std::vector<double> &args) {
      int winsize = std::floor(args[5]);
      int L2size = std::floor(args[2]);
      winsize = winsize % 2 ? winsize + 1 : winsize;
      L2size = L2size % 2 ? L2size : L2size + 1;
      IPCsettings ipcset_opt(winsize, winsize, args[0], args[1]);
      ipcset_opt.L2size = L2size;
      ipcset_opt.applyBandpass = args[3] > 0 ? true : false;
      ipcset_opt.applyWindow = args[4] > 0 ? true : false;
      ipcset_opt.speak = IPCsettings::None;
      FitsTime time_opt = time;
      DiffrotSettings drset_opt = drset;
      drset_opt.pics = 50;
      drset_opt.ys = 201;
      drset_opt.dPic = 1;
      drset_opt.pred = false;
      drset_opt.speak = false;
      return calculateDiffrotProfile(ipcset_opt, time_opt, drset_opt).GetError();
    };

    Evolution evo(6);
    evo.NP = 50;
    evo.mutStrat = Evolution::RAND1;
    evo.historyImprovTresholdPercent = 1;
    evo.lowerBounds = std::vector<double>{0.1, 1, 5, -1, -1, 128};
    evo.upperBounds = std::vector<double>{20, 500, 21, 1, 1, 512};
    auto result = evo.optimize(f);
    LOG_SUCC("Evolution result = {}", result);
  }

  if (1) // fixed window size
  {
    auto f = [&](const std::vector<double> &args) {
      int L2size = std::floor(args[2]);
      L2size = L2size % 2 ? L2size : L2size + 1;
      IPCsettings ipcset_opt(64, 64, args[0], args[1]);
      ipcset_opt.L2size = L2size;
      ipcset_opt.applyBandpass = args[3] > 0 ? true : false;
      ipcset_opt.applyWindow = args[4] > 0 ? true : false;
      ipcset_opt.speak = IPCsettings::None;
      FitsTime time_opt = time;
      DiffrotSettings drset_opt = drset;
      drset_opt.pics = 50;
      drset_opt.ys = 201;
      drset_opt.dPic = 1;
      drset_opt.pred = false;
      drset_opt.speak = false;
      return calculateDiffrotProfile(ipcset_opt, time_opt, drset_opt).GetError();
    };

    Evolution evo(5);
    evo.NP = 50;
    evo.mutStrat = Evolution::RAND1;
    evo.historyImprovTresholdPercent = 1;
    evo.lowerBounds = std::vector<double>{0.1, 1, 5, -1, -1};
    evo.upperBounds = std::vector<double>{20, 500, 21, 1, 1};
    auto result = evo.optimize(f);
    LOG_SUCC("Evolution result = {}", result);
  }
}

void WindowDiffrot::updateDrset()
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
  LOG_STARTEND("Creating diffrot video...", "Diffrot video created.");
  FitsTime time = GetStartFitsTime();

  drset.video = true;
  calculateDiffrotProfile(*globals->IPCset, time, drset);
  drset.video = false;
}

void WindowDiffrot::movingPeak()
{
  LOG_STARTEND("Creating moving diffrot peak images...", "Moving diffrot peak images created.");
  IPCsettings ipcset = *globals->IPCset;
  ipcset.save = true;
  ipcset.savedir = ui.lineEdit_9->text().toStdString();
  FitsImage pic1, pic2;
  int lag1, lag2;
  bool saveimgs = false;

  std::vector<double> dts;
  std::vector<std::vector<double>> shiftsX;

  const int profiles = 1;

  for (int profile = 0; profile < profiles; ++profile)
  {
    int sy = 0;
    LOG_DEBUG("profile {} ...", profile);
    shiftsX.push_back({});
    for (int dpic = 0; dpic < drset.pics; ++dpic)
    {
      FitsTime time = GetStartFitsTime();
      loadFitsFuzzy(pic1, time, lag1);
      time.advanceTime(dpic * drset.dSec);
      loadFitsFuzzy(pic2, time, lag2);

      if (pic1.params().succload && pic2.params().succload)
      {
        Mat crop1 = roicrop(pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY + sy, ipcset.getcols(), ipcset.getrows());
        Mat crop2 = roicrop(pic2.image(), pic2.params().fitsMidX, pic2.params().fitsMidY + sy, ipcset.getcols(), ipcset.getrows());
        auto shift = phasecorrel(std::move(crop1), std::move(crop2), ipcset, saveimgs);

        if (!profile)
          dts.push_back((double)dpic * drset.dSec / 60);

        shiftsX[profile].push_back(shift.x);
      }
    }
    profile++;
  }

  destroyAllWindows();

  Plot1D::plot(dts, shiftsX, "shiftsX", "time step [min]", "west-east image shift [px]", {}, Plot::defaultpens, ipcset.savedir + "plot.png");
}

FitsTime WindowDiffrot::GetStartFitsTime()
{
  updateDrset();
  return FitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
}
