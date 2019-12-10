#include "stdafx.h"
#include "WindowIPCoptimize.h"
#include "WindowIPCparameters.h"
#include "functionsAstro.h"

WindowIPCoptimize::WindowIPCoptimize(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
	ui.setupUi(this);
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(optimize()));
}

void WindowIPCoptimize::optimize()
{

	Evolution Evo(2);
	Evo.NP = 144;
	Evo.optimalFitness = 0;
	Evo.optimize([&](std::vector<double> arg) {return sqr(arg[0]) + sqr(arg[1]); }, globals->Logger);

	//optimizeIPCParameters(*globals->IPCsettings, ui.lineEdit->text().toStdString(), ui.lineEdit_2->text().toStdString(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_5->text().toInt(), globals->Logger);
	globals->Logger->LogMessage("IPC parameter optimization completed, see the results at\n" + ui.lineEdit_2->text().toStdString(), EVENT);
}