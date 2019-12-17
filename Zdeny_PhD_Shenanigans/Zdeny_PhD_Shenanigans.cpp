#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"

Zdeny_PhD_Shenanigans::Zdeny_PhD_Shenanigans(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	//show the logo
	QPixmap pm("logo.png"); // <- path to image file
	ui.label_2->setPixmap(pm);
	ui.label_2->setScaledContents(true);

	//allocate all globals - main window is loaded once only
	globals = new Globals();
	globals->IPCsettings = new IPCsettings();

	#ifdef LOGGER_QT
	globals->Logger = new QtLogger(g_loglevel, ui.textBrowser);
	#else
	globals->Logger = new CslLogger(g_loglevel);
	#endif

	globals->Logger->Log("Welcome back, my friend.", SPECIAL);//log a welcome message
	//create all windows
	windowIPCparameters = new WindowIPCparameters(this, globals);
	windowIPCoptimize = new WindowIPCoptimize(this, globals);
	windowIPC2PicAlign = new WindowIPC2PicAlign(this, globals);
	windowDiffrot = new WindowDiffrot(this, globals);

	//make signal to slot connections
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(exit()));
	connect(ui.actionAbout_Zdeny_s_PhD_Shenanigans, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.pushButtonAddLogs, SIGNAL(clicked()), this, SLOT(loggerToTextBrowser()));
	connect(ui.pushButtonClearLogs, SIGNAL(clicked()), this, SLOT(clearTextBrowser()));
	connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(closeCV()));
	connect(ui.actionIPC_parameters, SIGNAL(triggered()), this, SLOT(showWindowIPCparameters()));
	connect(ui.actionIPC_optimize, SIGNAL(triggered()), this, SLOT(showWindowIPCoptimize()));
	connect(ui.actionIPC_2pic_align, SIGNAL(triggered()), this, SLOT(showWindowIPC2PicAlign()));
	connect(ui.actionDebug, SIGNAL(triggered()), this, SLOT(debug()));
	connect(ui.actiondiffrot, SIGNAL(triggered()), this, SLOT(showWindowDiffrot()));
}

void Zdeny_PhD_Shenanigans::exit()
{
	QApplication::exit();
}

void Zdeny_PhD_Shenanigans::debug()
{

}

void Zdeny_PhD_Shenanigans::about()
{
	QMessageBox msgBox;
	msgBox.setText("All these shenanigans were created during my PhD studies.\n\nHave fun,\nZdenek Hrazdira\n2018-2020");
	msgBox.exec();
}

void Zdeny_PhD_Shenanigans::closeCV()
{
	destroyAllWindows();
	globals->Logger->Log("All image windows closed", INFO);
}

void Zdeny_PhD_Shenanigans::loggerToTextBrowser()
{
	for (int i = 0; i < 100; i++)
	{
		globals->Logger->Log("boziiinku, co ted budeme delat?", FATAL);
		globals->Logger->Log("jejda, nebude v tom nahodou nejaky probljhbemek?", EVENT);
		globals->Logger->Log("trololooo niga xdxd 5", SUBEVENT);
		globals->Logger->Log("objednal jsem ti ten novy monitor, jak jsi chtel", INFO);
		globals->Logger->Log("tak tenhle zapeklity problemek budeme muset", DEBUG);
	}
}

void Zdeny_PhD_Shenanigans::clearTextBrowser()
{
	ui.textBrowser->clear();
}

void Zdeny_PhD_Shenanigans::showWindowIPCparameters()
{
	windowIPCparameters->show();
}

void Zdeny_PhD_Shenanigans::showWindowIPCoptimize()
{
	windowIPCoptimize->show();
}

void Zdeny_PhD_Shenanigans::showWindowIPC2PicAlign()
{
	windowIPC2PicAlign->show();
}

void Zdeny_PhD_Shenanigans::showWindowDiffrot()
{
	windowDiffrot->show();
}
