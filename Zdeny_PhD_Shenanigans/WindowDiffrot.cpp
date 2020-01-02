#include "stdafx.h"
#include "WindowDiffrot.h"

WindowDiffrot::WindowDiffrot(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
	ui.setupUi(this);
	diffrotResults = new DiffrotResults;
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(calculateDiffrot()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(showResults()));
	connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(showIPC()));
	connect(ui.pushButton_4, SIGNAL(clicked()), this, SLOT(optimizeDiffrot()));
}

void WindowDiffrot::calculateDiffrot()
{
	globals->Logger->Log("Calculating diffrot profile...", EVENT);
	FITStime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	Plot1D plt(globals->widget);
	calculateDiffrotProfile(*globals->IPCsettings, fitsTime, diffrotResults, ui.lineEdit_7->text().toDouble(), ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(), ui.lineEdit_9->text().toStdString(), globals->Logger, &plt);
}

void WindowDiffrot::showIPC()
{
	globals->Logger->Log("Showimg diffrot profile IPC landscape...", EVENT);
	fitsParams params1, params2;
	IPCsettings set = *globals->IPCsettings;
	set.IPCshow = true;
	FITStime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	globals->Logger->Log("Loading file '" + fitsTime.path() + "'...", INFO);
	Mat pic1 = roicrop(loadfits(fitsTime.path(), params1), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows());
	fitsTime.advanceTime(ui.lineEdit_5->text().toDouble()*timeDelta);
	globals->Logger->Log("Loading file '" + fitsTime.path() + "'...", INFO);
	Mat pic2 = roicrop(loadfits(fitsTime.path(), params1), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows());

	auto shifts = phasecorrel(pic1, pic2, set, globals->Logger);
}

void WindowDiffrot::optimizeDiffrot()
{
	//optimize 
}

void WindowDiffrot::showResults()
{
	//diffrotResults->showResults();
}