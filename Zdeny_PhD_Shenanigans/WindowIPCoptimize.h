#pragma once
#include "ui_WindowIPCoptimize.h"
#include "globals.h"

class WindowIPCoptimize : public QMainWindow
{
	Q_OBJECT

public:
	WindowIPCoptimize(QWidget* parent, Globals* globals);

private:
	Ui::WindowIPCoptimize ui;
	Globals* globals;

private slots:
	void optimize();
};
