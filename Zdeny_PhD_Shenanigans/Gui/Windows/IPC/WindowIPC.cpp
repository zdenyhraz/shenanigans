#include "stdafx.h"
#include "WindowIPC.h"
#include "Astrophysics/SwindFlow.h"
#include "Plot/Plot2D.h"
#include "IPC/ipcAux.h"

WindowIPC::WindowIPC(QWidget *parent, Globals *globals) : QMainWindow(parent), globals(globals)
{
  ui.setupUi(this);
  RefreshIPCparameters(true);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(RefreshIPCparametersAndExit()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(RefreshIPCparameters()));
  connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(Optimize()));
  connect(ui.pushButton_9, SIGNAL(clicked()), this, SLOT(align()));
  connect(ui.pushButton_6, SIGNAL(clicked()), this, SLOT(alignXY()));
  connect(ui.pushButton_4, SIGNAL(clicked()), this, SLOT(flowMap()));
  connect(ui.pushButton_8, SIGNAL(clicked()), this, SLOT(features()));
  connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(linearFlow()));
  connect(ui.pushButton_7, SIGNAL(clicked()), this, SLOT(constantFlow()));
  connect(ui.pushButton_10, SIGNAL(clicked()), this, SLOT(ShowDebugStuff()));
}

void WindowIPC::RefreshIPCparameters(bool init)
{
  globals->IPC->SetBandpassType(static_cast<IterativePhaseCorrelation::BandpassType>(ui.comboBox->currentIndex()));
  globals->IPC->SetInterpolationType(static_cast<IterativePhaseCorrelation::InterpolationType>(ui.comboBox_2->currentIndex()));
  globals->IPC->SetWindowType(static_cast<IterativePhaseCorrelation::WindowType>(ui.comboBox_3->currentIndex()));
  globals->IPC->SetSize(ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt());
  globals->IPC->SetL2size(ui.lineEdit_2->text().toInt());
  globals->IPC->SetL1ratio(ui.lineEdit_3->text().toDouble());
  globals->IPC->SetUpsampleCoeff(ui.lineEdit_4->text().toInt());
  globals->IPC->SetBandpassParameters(ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble());

  if (!init)
    LOG_DEBUG("IPC parameter values updated");
}

void WindowIPC::RefreshIPCparametersAndExit()
{
  RefreshIPCparameters();
  hide();
}

void WindowIPC::Optimize()
{
  globals->IPC->Optimize(ui.lineEdit_10->text().toStdString(), ui.lineEdit_11->text().toStdString(), ui.lineEdit_12->text().toDouble(),
      ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt());
}

void WindowIPC::align()
{
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();
  Mat img1 = loadImage(path1);
  Mat img2 = loadImage(path2);

  int size = globals->IPCset->getcols();
  img1 = roicrop(img1, 0.5 * img1.cols, 0.5 * img1.rows, size, size);
  img2 = roicrop(img2, 0.5 * img2.cols, 0.5 * img2.rows, size, size);

  IPCsettings set = *globals->IPCset;
  set.speak = IPCsettings::All;

  if (0) // artificial misalign
  {
    img2 = img1.clone();
    double angle = 0;
    double scale = 1;
    double shiftX = 0;
    double shiftY = 0;
    Point2f center((float)img1.cols / 2, (float)img1.rows / 2);
    cout << "Artificial parameters:" << endl
         << "Angle: " << angle << endl
         << "Scale: " << scale << endl
         << "ShiftX: " << shiftX << endl
         << "ShiftY: " << shiftY << endl;
    Mat R = getRotationMatrix2D(center, angle, scale);
    Mat T = (Mat_<float>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
    warpAffine(img2, img2, T, cv::Size(img1.cols, img1.rows));
    warpAffine(img2, img2, R, cv::Size(img1.cols, img1.rows));
  }

  showimg(img1, "img1");
  showimg(img2, "img2");
  showimg(AlignStereovision(img1, img2), "img 1n2 not aligned");
  alignPics(img1, img2, img2, set);
  showimg(AlignStereovision(img1, img2), "img 1n2 yes aligned");
}

void WindowIPC::alignXY()
{
  LOG_DEBUG("Starting IPC image align process(XY)...");
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();
  Mat img1 = loadImage(path1);
  Mat img2 = loadImage(path2);

  int sizeX = globals->IPCset->getcols();
  int sizeY = globals->IPCset->getrows();
  img1 = roicrop(img1, ui.lineEdit_18->text().toDouble() * img1.cols, ui.lineEdit_19->text().toDouble() * img1.rows, sizeX, sizeY);
  img2 = roicrop(img2, ui.lineEdit_18->text().toDouble() * img2.cols, ui.lineEdit_19->text().toDouble() * img2.rows, sizeX, sizeY);

  IPCsettings set = *globals->IPCset;
  set.speak = IPCsettings::All;

  showimg(img1, "img1");
  showimg(img2, "img2");
  auto shifts = phasecorrel(img1, img2, set);
}

void WindowIPC::flowMap()
{
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();
  Mat img1 = loadImage(path1);
  Mat img2 = loadImage(path2);

  int size = 1024;
  img1 = roicrop(img1, 0.4 * img1.cols, 0.7 * img1.rows, size, size);
  img2 = roicrop(img2, 0.4 * img2.cols, 0.7 * img2.rows, size, size);

  showimg(img1, "img");
  Mat flowAD = abs(img1 - img2);
  showimg(flowAD, "flowAD", true, 0, 1);

  auto flowMap = calculateFlowMap(img1, img2, *globals->IPCset, 1);
  Mat flowX = std::get<0>(flowMap);
  Mat flowY = std::get<1>(flowMap);

  Mat flowM, flowP;
  magnitude(flowX, flowY, flowM);
  phase(flowX, flowY, flowP);

  showimg(flowX, "flowX", true);
  showimg(flowY, "flowY", true);
  showimg(flowM, "flowM", true);
  showimg(flowP, "flowP", true);
}

void WindowIPC::features()
{
  std::string path1 = ui.lineEdit_15->text().toStdString();
  std::string path2 = ui.lineEdit_16->text().toStdString();
  Mat img1 = loadImage(path1);
  Mat img2 = loadImage(path2);
}

void WindowIPC::linearFlow()
{
  LOG_DEBUG("Starting linear solar wind speed calculation...");
  auto xyi = calculateLinearSwindFlow(
      *globals->IPCset, ui.lineEdit_17->text().toStdString(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble());
  auto xshifts = std::get<0>(xyi);
  auto yshifts = std::get<1>(xyi);
  auto indices = std::get<2>(xyi);

  LOG_DEBUG("xshifts min = " + to_string(vectorMin(xshifts)));
  LOG_DEBUG("xshifts max = " + to_string(vectorMax(xshifts)));
  LOG_DEBUG("yshifts min = " + to_string(vectorMin(yshifts)));
  LOG_DEBUG("yshifts max = " + to_string(vectorMax(yshifts)));
  LOG_DEBUG("Linear solar wind speed calculated");
}

void WindowIPC::constantFlow()
{
  LOG_DEBUG("Starting constant solar wind speed calculation...");
  auto xyi = calculateConstantSwindFlow(
      *globals->IPCset, ui.lineEdit_17->text().toStdString(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble());
  auto xshifts = std::get<0>(xyi);
  auto yshifts = std::get<1>(xyi);
  auto indices = std::get<2>(xyi);

  LOG_DEBUG("xshifts min = " + to_string(vectorMin(xshifts)));
  LOG_DEBUG("xshifts max = " + to_string(vectorMax(xshifts)));
  LOG_DEBUG("yshifts min = " + to_string(vectorMin(yshifts)));
  LOG_DEBUG("yshifts max = " + to_string(vectorMax(yshifts)));
  LOG_DEBUG("Constant solar wind speed calculated");
}

void WindowIPC::ShowDebugStuff()
{
  RefreshIPCparameters();
  globals->IPC->ShowDebugStuff();
}
