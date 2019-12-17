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
	// configure axis rect:
	ui.widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
	ui.widget->axisRect()->setupFullAxesBox(true);
	ui.widget->xAxis->setLabel("x");
	ui.widget->yAxis->setLabel("y");

	// set up the QCPColorMap:
	QCPColorMap *colorMap = new QCPColorMap(ui.widget->xAxis, ui.widget->yAxis);
	int nx = 200;
	int ny = 200;
	colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
	colorMap->data()->setRange(QCPRange(-4, 4), QCPRange(-4, 4)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
	// now we assign some data, by accessing the QCPColorMapData instance of the color map:
	double x, y, z;
	for (int xIndex = 0; xIndex < nx; ++xIndex)
	{
		for (int yIndex = 0; yIndex < ny; ++yIndex)
		{
			colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
			double r = 3 * qSqrt(x*x + y * y) + 1e-2;
			z = 2 * x*(qCos(r + 2) / r - qSin(r + 2) / r); // the B field strength of dipole radiation (modulo physical constants)
			colorMap->data()->setCell(xIndex, yIndex, z);
		}
	}

	// add a color scale:
	QCPColorScale *colorScale = new QCPColorScale(ui.widget);
	ui.widget->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
	colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
	colorMap->setColorScale(colorScale); // associate the color map with the color scale
	colorScale->axis()->setLabel("Magnetic Field Strength");

	// set the color gradient of the color map to one of the presets:
	colorMap->setGradient(QCPColorGradient::gpPolar);
	// we could have also created a QCPColorGradient instance and added own colors to
	// the gradient, see the documentation of QCPColorGradient for what's possible.

	// rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
	colorMap->rescaleDataRange();

	// make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
	QCPMarginGroup *marginGroup = new QCPMarginGroup(ui.widget);
	ui.widget->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
	colorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

	// rescale the key (x) and value (y) axes so the whole color map is visible:
	ui.widget->rescaleAxes();
	ui.widget->replot();
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
