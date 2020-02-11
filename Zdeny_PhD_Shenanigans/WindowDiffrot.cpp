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
	connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(superOptimizeDiffrot()));
}

void WindowDiffrot::calculateDiffrot()
{
	globals->Logger->Log("Calculating diffrot profile...", EVENT);
	FITStime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	Plot1D pltX(globals->widget1);
	Plot1D pltY(globals->widget2);
	*diffrotResults = calculateDiffrotProfile(*globals->IPCsettings, fitsTime, ui.lineEdit_7->text().toDouble(), ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(), globals->Logger, &pltX, &pltY);
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
	fitsTime.advanceTime(ui.lineEdit_5->text().toDouble()*ui.lineEdit_8->text().toDouble());
	globals->Logger->Log("Loading file '" + fitsTime.path() + "'...", INFO);
	Mat pic2 = roicrop(loadfits(fitsTime.path(), params1), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows());

	auto shifts = phasecorrel(pic1, pic2, set, globals->Logger);
}

void WindowDiffrot::optimizeDiffrot()
{
	fitsParams params1, params2;
	FITStime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	std::vector<int> sizes{ 16,32,64,128 };
	std::string path = "D:\\MainOutput\\diffrot\\diffrotIPCopt.csv";
	std::ofstream listing(path, std::ios::out | std::ios::trunc);//just delete
	for (auto& size : sizes)
	{
		globals->Logger->Log("Optimizing IPC parameters for diffrot profile measurement " + to_string(size) + "x" + to_string(size) + "...", EVENT);
		IPCsettings set = *globals->IPCsettings;
		set.setSize(size, size);
		Plot1D* plt = new Plot1D(globals->widget1);
		optimizeIPCParameters(set, fitsTime.path(), path, 5, 0.01, 3, globals->Logger, plt);
		delete plt;
	}
	globals->Logger->Log("IPC parameters optimization for diffrot profile measurement finished", SUBEVENT);
}

void WindowDiffrot::superOptimizeDiffrot()
{
	Evolution Evo(5);
	Evo.NP = 30;
	Evo.mutStrat = Evolution::MutationStrategy::RAND1;
	Evo.lowerBounds = std::vector<double>{ 5,0,0,3,-1 };
	Evo.upperBounds = std::vector<double>{ 99,10,500,21,1 };
	globals->Logger->Log("Super optimizing diffrot profile...", EVENT);
	Plot1D* plt1 = new Plot1D(globals->widget1);
	Plot1D* plt2 = new Plot1D(globals->widget2);

	FITStime fitsTimeMaster(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	fitsTimeMaster.advanceTime(0);
	Mat pic1, pic2;
	fitsParams params1, params2;
	pic1 = loadfits(fitsTimeMaster.path(), params1);
	fitsTimeMaster.advanceTime(45);
	pic2 = loadfits(fitsTimeMaster.path(), params2);

	std::vector<std::pair<Mat, Mat>> pics{ std::pair<Mat, Mat>(pic1, pic2) };
	std::vector<std::pair<fitsParams, fitsParams>> params{ std::pair<fitsParams, fitsParams>(params1, params2) };

	auto f = [&](std::vector<double> arg) {return DiffrotMerritFunctionWrapper(arg, pics, params, ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(), plt2); };
	auto result = Evo.optimize(f, globals->Logger, plt1);
	globals->Logger->Log("Optimal Size: " + to_string(result[0]), INFO);
	globals->Logger->Log("Optimal Lmult: " + to_string(result[1]), INFO);
	globals->Logger->Log("Optimal Hmult: " + to_string(result[2]), INFO);
	globals->Logger->Log("Optimal L2size: " + to_string(result[3]), INFO);
	globals->Logger->Log("Optimal Window: " + to_string(result[4]), INFO);
	globals->Logger->Log("Super optimizing finished", SUBEVENT);
}

void WindowDiffrot::showResults()
{
	diffrotResults->showsaveResults(ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble(), ui.lineEdit_20->text().toDouble(), ui.lineEdit_16->text().toDouble(), ui.lineEdit_9->text().toStdString());
	globals->Logger->Log("Differential rotation results shown", EVENT);
}