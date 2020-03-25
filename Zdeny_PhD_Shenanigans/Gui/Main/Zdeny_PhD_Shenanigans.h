#pragma once
#include "ui_Zdeny_PhD_Shenanigans.h"
#include "Gui/Windows/WindowIPCparameters.h"
#include "Gui/Windows/WindowIPCoptimize.h"
#include "Gui/Windows/WindowIPC2PicAlign.h"
#include "Gui/Windows/WindowDiffrot.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"
#include "Log/logger.h"
#include "Procedural/procedural.h"
#include "Snake/game.h"

class Zdeny_PhD_Shenanigans : public QMainWindow
{
	Q_OBJECT

public:
	Zdeny_PhD_Shenanigans( QWidget *parent = Q_NULLPTR );

private:
	Ui::Zdeny_PhD_ShenanigansClass ui;
	Globals *globals;

	//windows
	WindowIPCparameters *windowIPCparameters;
	WindowIPCoptimize *windowIPCoptimize;
	WindowIPC2PicAlign *windowIPC2PicAlign;
	WindowDiffrot *windowDiffrot;

private slots:
	void exit();
	void about();
	void showWindowIPCparameters();
	void showWindowIPCoptimize();
	void showWindowIPC2PicAlign();
	void CloseAll();
	void debug();
	void showWindowDiffrot();
	void playSnake();
	void generateLand();
	void fitsDownloader();
	void fitsDownloadChecker();
	void closeEvent( QCloseEvent *event );
};
