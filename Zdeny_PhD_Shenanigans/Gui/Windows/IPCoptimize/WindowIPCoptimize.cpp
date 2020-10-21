#include "stdafx.h"
#include "Gui/Windows/IPCoptimize/WindowIPCoptimize.h"
#include "Gui/Windows/IPCparameters/WindowIPCparameters.h"
#include "Astrophysics/SDOipcOpt.h"
#include "Optimization/Evolution.h"

WindowIPCoptimize::WindowIPCoptimize(QWidget *parent, Globals *globals) : QMainWindow(parent), globals(globals)
{
  ui.setupUi(this);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(optimize()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(optimizeAll()));
}

void WindowIPCoptimize::optimize()
{
  if constexpr (0) // dbg
  {
    Evolution Evo(2);
    Evo.mNP = 32;
    Evo.mLB = zerovect(2, -100.);
    Evo.mUB = zerovect(2, +100.);
    Evo.Optimize([&](std::vector<double> arg) { return sin(sqr(arg[0]) - sqr(arg[1] - 3) + 6); });
  }
  optimizeIPCParameters(*globals->IPCset, ui.lineEdit->text().toStdString(), ui.lineEdit_2->text().toStdString(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_5->text().toInt());
  LOG_DEBUG("IPC parameter optimization completed, see the results at\n" + ui.lineEdit_2->text().toStdString());
}

void WindowIPCoptimize::optimizeAll() { optimizeIPCParametersForAllWavelengths(*globals->IPCset, ui.lineEdit_3->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_5->text().toInt()); }
