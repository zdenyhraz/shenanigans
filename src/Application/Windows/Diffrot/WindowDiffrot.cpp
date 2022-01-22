
#include "WindowDiffrot.h"

WindowDiffrot::WindowDiffrot(QWidget* parent, WindowData* windowData) : QMainWindow(parent), mWindowData(windowData)
{
  ui.setupUi(this);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(Calculate()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(Load()));
  connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(Optimize()));
  connect(ui.pushButton_4, SIGNAL(clicked()), this, SLOT(PlotMeridianCurve()));
}

DifferentialRotation WindowDiffrot::GetDifferentialRotation(i32 xsizeoverride)
{
  const i32 xsize = xsizeoverride ? xsizeoverride : ui.lineEdit->text().toInt();
  const i32 ysize = ui.lineEdit_9->text().toInt();
  const i32 idstep = ui.lineEdit_10->text().toInt();
  const i32 idstride = ui.lineEdit_11->text().toInt();
  const i32 yfov = ui.lineEdit_12->text().toInt();
  const i32 cadence = ui.lineEdit_13->text().toInt();
  return DifferentialRotation(xsize, ysize, idstep, idstride, yfov, cadence);
}

std::string WindowDiffrot::GetDataPath()
{
  return ui.lineEdit_7->text().toStdString();
}

i32 WindowDiffrot::GetIdstart()
{
  return ui.lineEdit_3->text().toInt();
}

void WindowDiffrot::Calculate()
{
  GetDifferentialRotation().Calculate(*mWindowData->IPC, GetDataPath(), GetIdstart());
}

void WindowDiffrot::Load()
{
  GetDifferentialRotation().LoadAndShow(ui.lineEdit_2->text().toStdString(), GetDataPath(), GetIdstart());
}

void WindowDiffrot::Optimize()
{
  const i32 xsizeopt = ui.lineEdit_6->text().toInt();
  const i32 ysizeopt = ui.lineEdit_14->text().toInt();
  const i32 popsize = ui.lineEdit_4->text().toInt();
  // const i32 maxgen = ui.lineEdit_5->text().toInt();
  GetDifferentialRotation().Optimize(*mWindowData->IPC, ui.lineEdit_8->text().toStdString(), GetIdstart(), xsizeopt, ysizeopt, popsize);
}

void WindowDiffrot::PlotMeridianCurve()
{
  const auto rawdata = GetDifferentialRotation(1).Calculate<true>(*mWindowData->IPC, GetDataPath(), GetIdstart());
  const auto [procdata, thetas] = DifferentialRotation::PostProcessData(rawdata);
  const auto timestep = ui.lineEdit_15->text().toDouble(); // [days]
  DifferentialRotation::PlotMeridianCurve(procdata, thetas, GetDataPath(), GetIdstart(), timestep);
}
