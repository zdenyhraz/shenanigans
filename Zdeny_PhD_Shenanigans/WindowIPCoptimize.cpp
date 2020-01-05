#include "stdafx.h"
#include "WindowIPCoptimize.h"
#include "WindowIPCparameters.h"
#include "functionsAstro.h"

WindowIPCoptimize::WindowIPCoptimize(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
	ui.setupUi(this);
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(optimize()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(optimizeAll()));
}

void WindowIPCoptimize::optimize()
{
	if (0)//dbg
	{
		Evolution Evo(2);
		Evo.NP = 32;
		Evo.lowerBounds = zerovect(2, -100);
		Evo.upperBounds = zerovect(2, +100);
		Evo.optimize([&](std::vector<double> arg) {return sin(sqr(arg[0]) - sqr(arg[1] - 3) + 6); }, globals->Logger);
	}
	Plot1D plt(globals->widget1);
	optimizeIPCParameters(*globals->IPCsettings, ui.lineEdit->text().toStdString(), ui.lineEdit_2->text().toStdString(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_5->text().toInt(), globals->Logger, &plt);
	globals->Logger->Log("IPC parameter optimization completed, see the results at\n" + ui.lineEdit_2->text().toStdString(), EVENT);
}

void WindowIPCoptimize::optimizeAll()
{
	optimizeIPCParametersForAllWavelengths(*globals->IPCsettings, ui.lineEdit_3->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_5->text().toInt(), globals->Logger);
}