#pragma once
#include "stdafx.h"
#include "functionsAstro.h"

static constexpr int plusminusbufer = 6;//even!

struct DiffrotSettings
{
	int pics;
	int ys;
	int strajdPic;
	int deltaPic;
	int verticalFov;
	int deltaSec;
};

DiffrotResults calculateDiffrotProfile(const IPCsettings& ipcset, FITStime& time, DiffrotSettings drset, Logger* logger, IPlot1D* plt1, IPlot1D* plt2)
{
	int dy = drset.verticalFov / (drset.ys - 1);
	int sy = 0;

	std::vector<std::vector<double>> omegasX;
	std::vector<double> shiftsXcurr(drset.ys);
	std::vector<double> omegasXcurr(drset.ys);
	std::vector<double> thetascurr(drset.ys);


	FitsImage pic1, pic2;

	for (int pic = 0; pic < drset.pics; pic++)
	{
		time.advanceTime((bool)pic*drset.deltaSec*(drset.strajdPic - drset.deltaPic));
		loadFitsFuzzy(pic1, time);
		time.advanceTime(drset.deltaPic*drset.deltaSec);
		loadFitsFuzzy(pic2, time);

		if (pic1.params().succload && pic2.params().succload)
		{
			double R = (pic1.params().R + pic2.params().R) / 2;
			double theta0 = (pic1.params().theta0 + pic2.params().theta0) / 2;

			calculateOmegas(pic1, pic2, shiftsXcurr, thetascurr, omegasXcurr, ipcset, drset, R, theta0, dy, sy);

			omegasX.emplace_back(omegasXcurr);

		}

		drplot();
	}

	theta2Dfit();
	return fillDiffrotResults();
}

void loadFitsFuzzy(FitsImage& pic, FITStime& time)
{
	pic.reload(time.path());

	if (!pic.params().succload)
	{
		for (int pm = 1; pm < plusminusbufer; pm++)
		{
			if (!(pm % 2))
				time.advanceTime(pm);
			else
				time.advanceTime(-pm);

			pic.reload(time.path());

			if (pic.params().succload)
				break;
		}
	}

	if (!pic.params().succload)
	{
		time.advanceTime(plusminusbufer / 2);
	}
}

void calculateOmegas(FitsImage& pic1, FitsImage& pic2, std::vector<double>& shiftsXcurr, std::vector<double>& thetascurr, std::vector<double>& omegasXcurr, const IPCsettings& ipcset, DiffrotSettings& drset, double R, double theta0, double dy, double sy)
{
	#pragma omp parallel for
	for (int y = 0; y < drset.ys; y++)
	{
		Mat crop1 = roicrop(pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY + dy * (y - drset.ys / 2) + sy, ipcset.getcols(), ipcset.getrows());
		Mat crop2 = roicrop(pic2.image(), pic2.params().fitsMidX, pic2.params().fitsMidY + dy * (y - drset.ys / 2) + sy, ipcset.getcols(), ipcset.getrows());
		shiftsXcurr[y] = phasecorrel(std::move(crop1), std::move(crop2), ipcset).x;
	}

	//filter outliers

	for (int y = 0; y < drset.ys; y++)
	{
		thetascurr[y] = asin(((double)dy*(drset.ys / 2 - y) - sy) / R) + theta0;
		omegasXcurr[y] = asin(shiftsXcurr[y] / (R*cos(thetascurr[y]))) / drset.deltaPic / drset.deltaSec;
	}
}

DiffrotResults fillDiffrotResults()
{
	return DiffrotResults();
}

void theta1Dfit()
{

}

void theta2Dfit()
{

}

void drplot()
{
}