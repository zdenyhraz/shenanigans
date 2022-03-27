
#include "WindowIPC.hpp"

WindowIPC::WindowIPC(QWidget* parent, WindowData* windowData) : QMainWindow(parent), mWindowData(windowData)
{
  ui.setupUi(this);
  RefreshIPCparameters();
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(RefreshIPCparametersAndExit()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(RefreshIPCparameters()));
  connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(Optimize()));
  connect(ui.pushButton_13, SIGNAL(clicked()), this, SLOT(PlotObjectiveFunctionLandscape()));
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
  mWindowData->mIPC.SetBandpassType(static_cast<IPC::BandpassType>(ui.comboBox->currentIndex()));
  mWindowData->mIPC.SetInterpolationType(static_cast<IPC::InterpolationType>(ui.comboBox_2->currentIndex()));
  mWindowData->mIPC.SetWindowType(static_cast<IPC::WindowType>(ui.comboBox_3->currentIndex()));
  mWindowData->mIPC.SetSize(ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt());
  mWindowData->mIPC.SetL2size(ui.lineEdit_2->text().toInt());
  mWindowData->mIPC.SetL1ratio(ui.lineEdit_3->text().toDouble());
  mWindowData->mIPC.SetL2Usize(ui.lineEdit_4->text().toInt());
  mWindowData->mIPC.SetBandpassParameters(ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble());
  mWindowData->mIPC.SetCrossPowerEpsilon(ui.lineEdit_7->text().toDouble());
  LOG_DEBUG("IPC parameters updated");
}

void WindowIPC::RefreshIPCparametersAndExit()
{
  RefreshIPCparameters();
  hide();
}

void WindowIPC::Optimize()
{
  RefreshIPCparameters();
  IPCOptimization::Optimize(mWindowData->mIPC, ui.lineEdit_10->text().toStdString(), ui.lineEdit_11->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(),
      ui.lineEdit_14->text().toInt(), ui.lineEdit_20->text().toDouble(), ui.lineEdit_21->text().toInt());
}

void WindowIPC::PlotObjectiveFunctionLandscape()
{
  RefreshIPCparameters();
  IPCOptimization::PlotObjectiveFunctionLandscape(
      mWindowData->mIPC, ui.lineEdit_10->text().toStdString(), ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt(), ui.lineEdit_22->text().toInt());
}

void WindowIPC::align()
{
  RefreshIPCparameters();
  // std::string path1 = ui.lineEdit_15->text().toStdString();
  // std::string path2 = ui.lineEdit_16->text().toStdString();

  cv::Point cropfocus(2048, 2048);
  i32 cropsize = 1.0 * 4096;

  cv::Mat img1 = RoiCrop(LoadUnitFloatImage<f32>("../data/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);
  cv::Mat img2 = RoiCrop(LoadUnitFloatImage<f32>("../data/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);

  i32 size = cropsize;
  cv::resize(img1, img1, cv::Size(size, size));
  cv::resize(img2, img2, cv::Size(size, size));

  Shift(img2, -950, 1050);
  Rotate(img2, 70, 1.2);

  IPC ipc = mWindowData->mIPC;
  ipc.SetSize(img1.size());
  cv::Mat aligned = IPCAlign::Align(ipc, img1, img2);
  Showimg(std::vector<cv::Mat>{img1, img2, aligned}, "align triplet");
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
    auto debugipc = mWindowData->mIPC;
    debugipc.Calculate(RoiCrop(img1, img1.cols / 2, img1.rows / 2, mWindowData->mIPC.GetCols(), mWindowData->mIPC.GetRows()),
        RoiCrop(img2, img2.cols / 2, img2.rows / 2, mWindowData->mIPC.GetCols(), mWindowData->mIPC.GetRows()));

    return;
  }

  const auto [flowX, flowY] = IPCFlow::CalculateFlow(mWindowData->mIPC, img1, img2, 0.125);
  cv::Mat flowM, flowP;
  cv::magnitude(flowX, flowY, flowM);
  cv::phase(flowX, flowY, flowP);
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
  // std::string path1 = ui.lineEdit_15->text().toStdString();
  // std::string path2 = ui.lineEdit_16->text().toStdString();
  // cv::Mat img1 = LoadUnitFloatImage<f32>(path1);
  // cv::Mat img2 = LoadUnitFloatImage<f32>(path2);
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
  IPCDebug::ShowDebugStuff(mWindowData->mIPC);
}
