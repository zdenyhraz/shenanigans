
#include "WindowIPC.h"
#include "Astrophysics/SwindFlow.h"
#include "Plot/Plot2D.h"
#include "IPC/ipcAux.h"

WindowIPC::WindowIPC(QWidget* parent, Globals* globals_) : QMainWindow(parent), globals(globals_)
{
  ui.setupUi(this);
  RefreshIPCparameters();
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(RefreshIPCparametersAndExit()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(RefreshIPCparameters()));
  connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(Optimize()));
  connect(ui.pushButton_13, SIGNAL(clicked()), this, SLOT(PlotObjectiveFunctionLandscape()));
  connect(ui.pushButton_12, SIGNAL(clicked()), this, SLOT(PlotUpsampleCoefficientAccuracyDependence()));
  connect(ui.pushButton_14, SIGNAL(clicked()), this, SLOT(PlotNoiseAccuracyDependence()));
  connect(ui.pushButton_15, SIGNAL(clicked()), this, SLOT(PlotNoiseOptimalBPHDependence()));
  connect(ui.pushButton_11, SIGNAL(clicked()), this, SLOT(PlotImageSizeAccuracyDependence()));
  connect(ui.pushButton_9, SIGNAL(clicked()), this, SLOT(align()));
  connect(ui.pushButton_6, SIGNAL(clicked()), this, SLOT(alignXY()));
  connect(ui.pushButton_4, SIGNAL(clicked()), this, SLOT(CalculateFlow()));
  connect(ui.pushButton_8, SIGNAL(clicked()), this, SLOT(features()));
  connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(linearFlow()));
  connect(ui.pushButton_7, SIGNAL(clicked()), this, SLOT(constantFlow()));
  connect(ui.pushButton_10, SIGNAL(clicked()), this, SLOT(ShowDebugStuff()));
}

void WindowIPC::RefreshIPCparameters()
{
  globals->IPC->SetBandpassType(static_cast<IterativePhaseCorrelation<>::BandpassType>(ui.comboBox->currentIndex()));
  globals->IPC->SetInterpolationType(static_cast<IterativePhaseCorrelation<>::InterpolationType>(ui.comboBox_2->currentIndex()));
  globals->IPC->SetWindowType(static_cast<IterativePhaseCorrelation<>::WindowType>(ui.comboBox_3->currentIndex()));
  globals->IPC->SetSize(ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt());
  globals->IPC->SetL2size(ui.lineEdit_2->text().toInt());
  globals->IPC->SetL1ratio(ui.lineEdit_3->text().toDouble());
  globals->IPC->SetUpsampleCoeff(ui.lineEdit_4->text().toInt());
  globals->IPC->SetBandpassParameters(ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble());
}

void WindowIPC::RefreshIPCparametersAndExit()
{
  RefreshIPCparameters();
  hide();
}

void WindowIPC::Optimize()
{
  RefreshIPCparameters();
  globals->IPC->Optimize(ui.lineEdit_10->text().toStdString(), ui.lineEdit_11->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(),
      ui.lineEdit_14->text().toInt(), ui.lineEdit_20->text().toDouble(), ui.lineEdit_21->text().toInt());
}

void WindowIPC::PlotObjectiveFunctionLandscape()
{
  RefreshIPCparameters();
  globals->IPC->PlotObjectiveFunctionLandscape(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotUpsampleCoefficientAccuracyDependence()
{
  RefreshIPCparameters();
  globals->IPC->PlotUpsampleCoefficientAccuracyDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotNoiseAccuracyDependence()
{
  RefreshIPCparameters();
  globals->IPC->PlotNoiseAccuracyDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotNoiseOptimalBPHDependence()
{
  RefreshIPCparameters();
  globals->IPC->PlotNoiseOptimalBPHDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotImageSizeAccuracyDependence()
{
  RefreshIPCparameters();
  globals->IPC->PlotImageSizeAccuracyDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::align()
{
  RefreshIPCparameters();
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();

  cv::Point cropfocus(2048, 2048);
  i32 cropsize = 1.0 * 4096;

  cv::Mat img1 = roicrop(loadImage("Resources/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);
  cv::Mat img2 = roicrop(loadImage("Resources/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);

  i32 size = cropsize;
  cv::resize(img1, img1, cv::Size(size, size));
  cv::resize(img2, img2, cv::Size(size, size));

  Shift(img2, -950, 1050);
  Rotate(img2, 70, 1.2);

  IterativePhaseCorrelation ipc = *globals->IPC;
  ipc.SetSize(img1.size());
  cv::Mat aligned = ipc.Align(img1, img2);
  showimg(std::vector<cv::Mat>{img1, img2, aligned}, "align triplet");
}

void WindowIPC::alignXY()
{
  LOG_DEBUG("Starting IPC image align process(XY)...");
  RefreshIPCparameters();
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();
  cv::Mat img1 = loadImage(path1);
  cv::Mat img2 = loadImage(path2);

  i32 sizeX = globals->IPCset->getcols();
  i32 sizeY = globals->IPCset->getrows();
  img1 = roicrop(img1, ui.lineEdit_18->text().toDouble() * img1.cols, ui.lineEdit_19->text().toDouble() * img1.rows, sizeX, sizeY);
  img2 = roicrop(img2, ui.lineEdit_18->text().toDouble() * img2.cols, ui.lineEdit_19->text().toDouble() * img2.rows, sizeX, sizeY);

  IPCsettings set = *globals->IPCset;
  set.speak = IPCsettings::All;

  showimg(img1, "img1");
  showimg(img2, "img2");
  phasecorrel(img1, img2, set);
}

void WindowIPC::CalculateFlow()
{
  RefreshIPCparameters();
  const auto path1 = ui.lineEdit_15->text().toStdString();
  const auto path2 = ui.lineEdit_16->text().toStdString();
  cv::Mat img1 = loadImage(path1);
  cv::Mat img2 = loadImage(path2);

  if (0)
  {
    auto debugipc = *globals->IPC;
    debugipc.Calculate(
        roicrop(img1, img1.cols / 2, img1.rows / 2, globals->IPC->GetCols(), globals->IPC->GetRows()), roicrop(img2, img2.cols / 2, img2.rows / 2, globals->IPC->GetCols(), globals->IPC->GetRows()));

    return;
  }

  const auto [flowX, flowY] = globals->IPC->CalculateFlow(img1, img2, 0.125);
  cv::Mat flowM, flowP;
  magnitude(flowX, flowY, flowM);
  phase(flowX, flowY, flowP);
  flowM *= 696010. / 378.3 / 11.8;
  flowP *= Constants::Rad;

  for (i32 r = 0; r < flowM.rows; ++r)
    for (i32 c = 0; c < flowM.cols; ++c)
      flowM.at<f32>(r, c) = std::clamp(flowM.at<f32>(r, c), 0.f, 800.f);

  Plot2D::Plot("FlowM", flowM);
  Plot2D::Plot("FlowP", flowP);
}

void WindowIPC::features()
{
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();
  cv::Mat img1 = loadImage(path1);
  cv::Mat img2 = loadImage(path2);
}

void WindowIPC::linearFlow()
{
  LOG_DEBUG("Starting linear solar wind speed calculation...");
  auto xyi = calculateLinearSwindFlow(*globals->IPCset, ui.lineEdit_17->text().toStdString(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble());
  auto xshifts = std::get<0>(xyi);
  auto yshifts = std::get<1>(xyi);
  auto indices = std::get<2>(xyi);

  LOG_DEBUG("xshifts min = " + std::to_string(vectorMin(xshifts)));
  LOG_DEBUG("xshifts max = " + std::to_string(vectorMax(xshifts)));
  LOG_DEBUG("yshifts min = " + std::to_string(vectorMin(yshifts)));
  LOG_DEBUG("yshifts max = " + std::to_string(vectorMax(yshifts)));
  LOG_DEBUG("Linear solar wind speed calculated");
}

void WindowIPC::constantFlow()
{
  LOG_DEBUG("Starting constant solar wind speed calculation...");
  auto xyi = calculateConstantSwindFlow(*globals->IPCset, ui.lineEdit_17->text().toStdString(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble());
  auto xshifts = std::get<0>(xyi);
  auto yshifts = std::get<1>(xyi);
  auto indices = std::get<2>(xyi);

  LOG_DEBUG("xshifts min = " + std::to_string(vectorMin(xshifts)));
  LOG_DEBUG("xshifts max = " + std::to_string(vectorMax(xshifts)));
  LOG_DEBUG("yshifts min = " + std::to_string(vectorMin(yshifts)));
  LOG_DEBUG("yshifts max = " + std::to_string(vectorMax(yshifts)));
  LOG_DEBUG("Constant solar wind speed calculated");
}

void WindowIPC::ShowDebugStuff()
{
  RefreshIPCparameters();
  globals->IPC->ShowDebugStuff();
}
