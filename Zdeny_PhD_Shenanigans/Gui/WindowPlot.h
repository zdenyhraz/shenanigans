#pragma once
#include "ui_WindowPlot.h"

class WindowPlot : public QMainWindow
{
	Q_OBJECT

public:
	WindowPlot();
	~WindowPlot();
	Ui::WindowPlot ui;

private:
	void closeEvent( QCloseEvent *event );

private slots:

};
