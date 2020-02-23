#include "stdafx.h"
#include "WindowDiffrot.h"
#include "Astrophysics/diffrot.h"

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
	LOG_DEBUG("Calculating diffrot profile...");
	FitsTime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	Plot1D pltX(globals->widget1);
	Plot1D pltY(globals->widget2);
	//*diffrotResults = calculateDiffrotProfile(*globals->IPCset, fitsTime, ui.lineEdit_7->text().toDouble(), ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(),  &pltX, &pltY);

	DiffrotSettings drset;
	drset.pics = ui.lineEdit_7->text().toDouble();
	drset.ys = ui.lineEdit_2->text().toDouble();
	drset.sPic = ui.lineEdit_6->text().toDouble();
	drset.dPic = ui.lineEdit_5->text().toDouble();
	drset.vFov = ui.lineEdit_4->text().toDouble();
	drset.dSec = ui.lineEdit_8->text().toDouble();

	//globals->IPCset->IPCshow = true;

	*diffrotResults = calculateDiffrotProfile(*globals->IPCset, fitsTime, drset,  &pltX, &pltY);
}

void WindowDiffrot::showIPC()
{
	LOG_DEBUG("Showimg diffrot profile IPC landscape...");
	FitsParams params1, params2;
	IPCsettings set = *globals->IPCset;
	set.IPCshow = true;
	FitsTime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	LOG_DEBUG("Loading file '" + fitsTime.path() + "'...");
	Mat pic1 = roicrop(loadfits(fitsTime.path(), params1), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows());
	fitsTime.advanceTime(ui.lineEdit_5->text().toDouble()*ui.lineEdit_8->text().toDouble());
	LOG_DEBUG("Loading file '" + fitsTime.path() + "'...");
	Mat pic2 = roicrop(loadfits(fitsTime.path(), params1), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows());

	auto shifts = phasecorrel(pic1, pic2, set);
}

void WindowDiffrot::optimizeDiffrot()
{
	FitsParams params1, params2;
	FitsTime fitsTime(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());
	std::vector<int> sizes{ 16,32,64,128 };
	std::string path = "D:\\MainOutput\\diffrot\\diffrotIPCopt.csv";
	std::ofstream listing(path, std::ios::out | std::ios::trunc);//just delete
	for (auto& size : sizes)
	{
		LOG_DEBUG("Optimizing IPC parameters for diffrot profile measurement " + to_string(size) + "x" + to_string(size) + "...");
		IPCsettings set = *globals->IPCset;
		set.setSize(size, size);
		Plot1D* plt = new Plot1D(globals->widget1);
		optimizeIPCParameters(set, fitsTime.path(), path, 5, 0.01, 3,  plt);
		delete plt;
	}
	LOG_DEBUG("IPC parameters optimization for diffrot profile measurement finished");
}

void WindowDiffrot::superOptimizeDiffrot()
{
	Evolution Evo(5);
	Evo.NP = 50;
	Evo.mutStrat = Evolution::MutationStrategy::BEST1;
	Evo.lowerBounds = std::vector<double>{ 5,0,0,3,-1 };
	Evo.upperBounds = std::vector<double>{ 101,10,500,21,1 };
	LOG_DEBUG("Super optimizing diffrot profile...");
	Plot1D* plt1 = new Plot1D(globals->widget1);
	Plot1D* plt2 = new Plot1D(globals->widget2);

	FitsTime fitsTimeMaster(ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt());

	std::vector<std::pair<FitsImage, FitsImage>> pics;
	pics.reserve(20);

	for (int i = 0; i < 3; i++)
	{
		FitsImage a(fitsTimeMaster.path());
		fitsTimeMaster.advanceTime(45);

		FitsImage b(fitsTimeMaster.path());

		if (a.params().succload && b.params().succload)
			pics.emplace_back(std::make_pair(a, b));

		//showimg(a.image(), "a" + to_string(i));
		//showimg(b.image(), "b" + to_string(i));
	}

	auto f = [&](std::vector<double> arg) {return DiffrotMerritFunctionWrapper(arg, pics, ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(), plt2); };
	std::vector<double> test{ 53,3.5,123.3,11,0.7 };
	LOG_DEBUG("Opt fn consistency test1=" + to_string(f(test)));
	LOG_DEBUG("Opt fn consistency test1=" + to_string(f(test)));
	LOG_DEBUG("Opt fn consistency test1=" + to_string(f(test)));
	auto result = Evo.optimize(f,  plt1);
	LOG_DEBUG("Optimal Size: " + to_string(result[0]));
	LOG_DEBUG("Optimal Lmult: " + to_string(result[1]));
	LOG_DEBUG("Optimal Hmult: " + to_string(result[2]));
	LOG_DEBUG("Optimal L2size: " + to_string(result[3]));
	LOG_DEBUG("Optimal Window: " + to_string(result[4]));
	LOG_DEBUG("Super optimizing finished");
}

void WindowDiffrot::showResults()
{
	diffrotResults->showsaveResults(ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble(), ui.lineEdit_20->text().toDouble(), ui.lineEdit_16->text().toDouble(), ui.lineEdit_9->text().toStdString());
	LOG_DEBUG("Differential rotation results shown");
}