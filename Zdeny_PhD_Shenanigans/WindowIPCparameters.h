#pragma once
#include "ui_WindowIPCparameters.h"
#include "globals.h"

class WindowIPCparameters : public QMainWindow
{
	Q_OBJECT

public:
	WindowIPCparameters(QWidget* parent, Globals* globals);

private:
	Ui::WindowIPCparameters ui;
	Globals* globals;

private slots:
	void refreshIPCparameters();
	void refreshIPCparametersAndExit();
};
