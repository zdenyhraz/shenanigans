#include "stdafx.h"
#include "WindowIPCparameters.h"

WindowIPCparameters::WindowIPCparameters(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
	ui.setupUi(this);
	this->refreshIPCparameters();
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(refreshIPCparametersAndExit()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(refreshIPCparameters()));
}

void WindowIPCparameters::refreshIPCparameters()
{
	globals->IPCsettings->setSize(ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt());
	globals->IPCsettings->L2size = ui.lineEdit_2->text().toInt();
	globals->IPCsettings->L1ratio = ui.lineEdit_3->text().toDouble();
	globals->IPCsettings->UC = ui.lineEdit_4->text().toInt();
	globals->IPCsettings->setBandpassParameters(ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble());
	globals->IPCsettings->epsilon = ui.lineEdit_7->text().toDouble();
	globals->IPCsettings->minimalShift = ui.lineEdit_8->text().toDouble();

	globals->IPCsettings->subpixel = ui.checkBox->isChecked();
	globals->IPCsettings->iterate = ui.checkBox_2->isChecked();
	globals->IPCsettings->applyBandpass = ui.checkBox_3->isChecked();
	globals->IPCsettings->applyWindow = ui.checkBox_4->isChecked();
	globals->IPCsettings->interpolate = ui.checkBox_5->isChecked();
	globals->IPCsettings->normInput = ui.checkBox_6->isChecked();
	globals->IPCsettings->crossCorrel = ui.checkBox_7->isChecked();

	globals->Logger->Log("IPC parameter values updated", INFO);
}

void WindowIPCparameters::refreshIPCparametersAndExit()
{
	this->refreshIPCparameters();
	this->hide();
}
