#pragma once
#include "ui_WindowDiffrot.h"
#include "globals.h"

class WindowDiffrot : public QMainWindow
{
	Q_OBJECT

public:
	WindowDiffrot(QWidget* parent, Globals* globals);

private:
	Ui::WindowDiffrot ui;
	Globals* globals;

private slots:

};
