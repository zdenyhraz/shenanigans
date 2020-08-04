#pragma once
#include "ui_Zdeny_PhD_Shenanigans.h"
#include "Gui/Windows/IPCparameters/WindowIPCparameters.h"
#include "Gui/Windows/IPCoptimize/WindowIPCoptimize.h"
#include "Gui/Windows/IPC2PicAlign/WindowIPC2PicAlign.h"
#include "Gui/Windows/Diffrot/WindowDiffrot.h"
#include "Gui/Windows/Features/WindowFeatures.h"

class Zdeny_PhD_Shenanigans : public QMainWindow
{
	Q_OBJECT

public:
	Zdeny_PhD_Shenanigans( QWidget *parent = Q_NULLPTR );

private:
	Ui::Zdeny_PhD_ShenanigansClass ui;
	std::unique_ptr<Globals> globals;

	//windows
	std::unique_ptr<WindowIPCparameters> windowIPCparameters;
	std::unique_ptr<WindowIPCoptimize> windowIPCoptimize;
	std::unique_ptr<WindowIPC2PicAlign> windowIPC2PicAlign;
	std::unique_ptr<WindowDiffrot> windowDiffrot;
	std::unique_ptr<WindowFeatures> windowFeatures;

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
	void featureMatch();
};
