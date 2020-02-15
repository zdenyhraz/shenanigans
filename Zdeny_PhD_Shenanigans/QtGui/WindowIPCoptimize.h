#pragma once
#include "ui_WindowIPCoptimize.h"
#include "Core/globals.h"

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
	void optimizeAll();
};
