#include "stdafx.h"
#include "WindowIPC.h"

WindowIPC::WindowIPC(QWidget *parent, Globals *globals) : QMainWindow(parent), globals(globals)
{
  ui.setupUi(this);
  RefreshIPCparameters(true);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(RefreshIPCparametersAndExit()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(RefreshIPCparameters()));
  connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(Optimize()));
}

void WindowIPC::RefreshIPCparameters(bool init)
{
  globals->IPC->SetSize(ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt());
  globals->IPC->SetL2size(ui.lineEdit_2->text().toInt());
  globals->IPC->SetL1ratio(ui.lineEdit_3->text().toDouble());
  globals->IPC->SetUpsampleCoeff(ui.lineEdit_4->text().toInt());
  globals->IPC->SetBandpassParameters(ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble());
  globals->IPC->SetDivisionEpsilon(ui.lineEdit_7->text().toDouble());
  globals->IPC->SetSubpixelEstimation(ui.checkBox->isChecked());
  globals->IPC->SetApplyBandpass(ui.checkBox_3->isChecked());
  globals->IPC->SetApplyWindow(ui.checkBox_4->isChecked());
  globals->IPC->SetCrossCorrelate(ui.checkBox_7->isChecked());

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
  // optimize ipc xdd
  globals->IPC->Optimize(std::vector<Mat>{}, ui.lineEdit_12->text().toDouble(), ui.lineEdit_13->text().toDouble(), ui.lineEdit_14->text().toInt());
}
