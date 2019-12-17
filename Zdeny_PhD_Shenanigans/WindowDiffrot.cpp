#include "stdafx.h"
#include "WindowDiffrot.h"

WindowDiffrot::WindowDiffrot(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
	ui.setupUi(this);
	diffrotResults = new DiffrotResults;
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(calculateDiffrot()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(showResults()));
}

void WindowDiffrot::calculateDiffrot()
{
	globals->Logger->Log("Calculating diffrot profile...", EVENT);
	FITStime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	globals->Logger->Log("Starting image path: " + fitsTime.path(), SUBEVENT);
	calculateDiffrotProfile(*globals->IPCsettings, *globals->IPCsettings, *globals->IPCsettings, fitsTime, diffrotResults, 0, ui.lineEdit_7->text().toDouble(), ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(), ui.lineEdit_9->text().toStdString());
}

void WindowDiffrot::showResults()
{
	//diffrotResults->showResults();
}