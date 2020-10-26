#include "stdafx.h"
#include "WindowIPC.h"

WindowIPC::WindowIPC(QWidget *parent, Globals *globals) : QMainWindow(parent), globals(globals)
{
  ui.setupUi(this);
  refreshIPCparameters(true);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(refreshIPCparametersAndExit()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(refreshIPCparameters()));
}

void WindowIPC::refreshIPCparameters(bool init)
{
  globals->IPCset->setSize(ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt());
  globals->IPCset->L2size = ui.lineEdit_2->text().toInt();
  globals->IPCset->L1ratio = ui.lineEdit_3->text().toDouble();
  globals->IPCset->UC = ui.lineEdit_4->text().toInt();
  globals->IPCset->setBandpassParameters(ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble());
  globals->IPCset->epsilon = ui.lineEdit_7->text().toDouble();
  globals->IPCset->minimalShift = ui.lineEdit_8->text().toDouble();
  globals->IPCset->subpixel = ui.checkBox->isChecked();
  globals->IPCset->applyBandpass = ui.checkBox_3->isChecked();
  globals->IPCset->applyWindow = ui.checkBox_4->isChecked();
  globals->IPCset->interpolate = ui.checkBox_5->isChecked();
  globals->IPCset->crossCorrel = ui.checkBox_7->isChecked();

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

void WindowIPC::refreshIPCparametersAndExit()
{
  refreshIPCparameters();
  hide();
}
