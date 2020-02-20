#pragma once
#include "ui_Zdeny_PhD_Shenanigans.h"
#include "WindowIPCparameters.h"
#include "WindowIPCoptimize.h"
#include "WindowIPC2PicAlign.h"
#include "WindowDiffrot.h"
#include "Plot/plotterqt.h"

class Zdeny_PhD_Shenanigans : public QMainWindow
{
	Q_OBJECT

public:
	Zdeny_PhD_Shenanigans(QWidget *parent = Q_NULLPTR);

private:
	Ui::Zdeny_PhD_ShenanigansClass ui;
	Globals* globals;

	//windows
	WindowIPCparameters* windowIPCparameters;
	WindowIPCoptimize* windowIPCoptimize;
	WindowIPC2PicAlign* windowIPC2PicAlign;
	WindowDiffrot* windowDiffrot;

private slots:
	void exit();
	void about();
	void loggerToTextBrowser();
	void showWindowIPCparameters();
	void showWindowIPCoptimize();
	void showWindowIPC2PicAlign();
	void closeCV();
	void debug();
	void showWindowDiffrot();
	void playSnake();
	void generateLand();
};
