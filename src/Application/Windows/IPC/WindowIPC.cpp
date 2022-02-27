
#include "WindowIPC.h"

WindowIPC::WindowIPC(QWidget* parent, WindowData* windowData) : QMainWindow(parent), mWindowData(windowData)
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
  mWindowData->IPC->SetBandpassType(static_cast<IterativePhaseCorrelation<>::BandpassType>(ui.comboBox->currentIndex()));
  mWindowData->IPC->SetInterpolationType(static_cast<IterativePhaseCorrelation<>::InterpolationType>(ui.comboBox_2->currentIndex()));
  mWindowData->IPC->SetWindowType(static_cast<IterativePhaseCorrelation<>::WindowType>(ui.comboBox_3->currentIndex()));
  mWindowData->IPC->SetSize(ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt());
  mWindowData->IPC->SetL2size(ui.lineEdit_2->text().toInt());
  mWindowData->IPC->SetL1ratio(ui.lineEdit_3->text().toDouble());
  mWindowData->IPC->SetUpsampleCoeff(ui.lineEdit_4->text().toInt());
  mWindowData->IPC->SetBandpassParameters(ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble());
}

void WindowIPC::RefreshIPCparametersAndExit()
{
  RefreshIPCparameters();
  hide();
}

void WindowIPC::Optimize()
{
  RefreshIPCparameters();
  mWindowData->IPC->Optimize(ui.lineEdit_10->text().toStdString(), ui.lineEdit_11->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(),
      ui.lineEdit_14->text().toInt(), ui.lineEdit_20->text().toDouble(), ui.lineEdit_21->text().toInt());
}

void WindowIPC::PlotObjectiveFunctionLandscape()
{
  RefreshIPCparameters();
  mWindowData->IPC->PlotObjectiveFunctionLandscape(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotUpsampleCoefficientAccuracyDependence()
{
  RefreshIPCparameters();
  mWindowData->IPC->PlotUpsampleCoefficientAccuracyDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotNoiseAccuracyDependence()
{
  RefreshIPCparameters();
  mWindowData->IPC->PlotNoiseAccuracyDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotNoiseOptimalBPHDependence()
{
  RefreshIPCparameters();
  mWindowData->IPC->PlotNoiseOptimalBPHDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::PlotImageSizeAccuracyDependence()
{
  RefreshIPCparameters();
  mWindowData->IPC->PlotImageSizeAccuracyDependence(
      ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::align()
{
  RefreshIPCparameters();
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();

  cv::Point cropfocus(2048, 2048);
  i32 cropsize = 1.0 * 4096;

  cv::Mat img1 = RoiCrop(LoadUnitFloatImage<f32>("../data/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);
  cv::Mat img2 = RoiCrop(LoadUnitFloatImage<f32>("../data/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);

  i32 size = cropsize;
  cv::resize(img1, img1, cv::Size(size, size));
  cv::resize(img2, img2, cv::Size(size, size));

  Shift(img2, -950, 1050);
  Rotate(img2, 70, 1.2);

  IterativePhaseCorrelation ipc = *mWindowData->IPC;
  ipc.SetSize(img1.size());
  cv::Mat aligned = ipc.Align(img1, img2);
  showimg(std::vector<cv::Mat>{img1, img2, aligned}, "align triplet");
}

void WindowIPC::alignXY()
{
}

void WindowIPC::CalculateFlow()
{
  RefreshIPCparameters();
  const auto path1 = ui.lineEdit_15->text().toStdString();
  const auto path2 = ui.lineEdit_16->text().toStdString();
  cv::Mat img1 = LoadUnitFloatImage<f32>(path1);
  cv::Mat img2 = LoadUnitFloatImage<f32>(path2);

  if (0)
  {
    auto debugipc = *mWindowData->IPC;
    debugipc.Calculate(RoiCrop(img1, img1.cols / 2, img1.rows / 2, mWindowData->IPC->GetCols(), mWindowData->IPC->GetRows()),
        RoiCrop(img2, img2.cols / 2, img2.rows / 2, mWindowData->IPC->GetCols(), mWindowData->IPC->GetRows()));

    return;
  }

  const auto [flowX, flowY] = mWindowData->IPC->CalculateFlow(img1, img2, 0.125);
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
  cv::Mat img1 = LoadUnitFloatImage<f32>(path1);
  cv::Mat img2 = LoadUnitFloatImage<f32>(path2);
}

void WindowIPC::linearFlow()
{
}

void WindowIPC::constantFlow()
{
}

void WindowIPC::ShowDebugStuff()
{
  RefreshIPCparameters();
  mWindowData->IPC->ShowDebugStuff();
}
