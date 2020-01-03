#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"
#include "FITS.h"
#include "IPC.h"
#include "optimization.h"
#include "logger.h"

using namespace std;
using namespace cv;

static const std::vector<string> WAVELENGTHS_STR = { "HMI", "94_AIA", "131_AIA", "171_AIA", "171_SECCHIA", "171_SECCHIB", "193_AIA", "195_SECCHIA", "195_SECCHIB", "211_AIA", "284_SECCHIA", "284_SECCHIB", "304_AIA", "304_SECCHIA", "304_SECCHIB", "335_AIA" };
static const std::vector<double> STDDEVS(WAVELENGTHS_STR.size(), 0);
static const int SwindPicCnt = 10;

std::vector<double> diffrotProfileAverage(const Mat& flow, int colS = 0);

struct DiffrotResults
{
	Mat FlowPic;
	Mat FlowX;
	Mat FlowY;
	std::vector<double> FlowXfit;

	void showResults(double quantileBot, double quantileTop, int medianSize, double sigma)
	{
		//first compute the X/Y abs/rel median pics
		Mat medFlowX = FlowX.clone();
		Mat medFlowY = FlowY.clone();
		medFlowX.convertTo(medFlowX, CV_32F);
		medFlowY.convertTo(medFlowY, CV_32F);
		for (int med = 3; med <= min(medianSize, 7); med += 2)
		{
			medianBlur(medFlowX, medFlowX, med);
			medianBlur(medFlowY, medFlowY, med);
		}
		medFlowX.convertTo(medFlowX, CV_64F);
		medFlowY.convertTo(medFlowY, CV_64F);
		Mat medFlowXrelative = medFlowX - matFromVector(FlowXfit, medFlowX.cols);

		//now show various stuff
		if (1)//source
		{
			showimg(FlowPic, "Flow Source", false, 0, 1);
		}
		if (1)//flow
		{
			showimg(medFlowX, "Flow X", true, quantileBot, quantileTop);
			showimg(medFlowY, "Flow Y", true, quantileBot, quantileTop);
		}
		if (1)//relative flow
		{
			showimg(medFlowXrelative, "Flow relative", true, quantileBot, quantileTop);
		}
		if (1)//ghetto sum
		{
			Mat FlowPicBGR = FlowPic.clone();
			FlowPicBGR.convertTo(FlowPicBGR, CV_32F);
			cvtColor(FlowPicBGR, FlowPicBGR, CV_GRAY2BGR);
			FlowPicBGR.convertTo(FlowPicBGR, CV_64F);
			normalize(FlowPicBGR, FlowPicBGR, 0, 1, CV_MINMAX);
			Mat medFlowX_BGR = applyColorMapZdeny(medFlowX, quantileBot, quantileTop);
			medFlowX_BGR.convertTo(medFlowX_BGR, CV_64F);
			normalize(medFlowX_BGR, medFlowX_BGR, 0, 1, CV_MINMAX);
			Mat mergedGHE = (1. - sigma)*FlowPicBGR + sigma * medFlowX_BGR;
			showimg(mergedGHE, "Flow Merged Ghetto", false, quantileBot, quantileTop);
		}
		if (0)//HUE sum
		{
			Mat mergedHUE = combineTwoPics(applyColorMapZdeny(medFlowX, quantileBot, quantileTop, false), FlowPic, HUEBRIGHT);
			showimg(mergedHUE, "Flow Merged Hue", false, quantileBot, quantileTop);
		}
		if (0)//HUE sum relative
		{
			Mat mergedHUE = combineTwoPics(medFlowXrelative, FlowPic, HUEBRIGHT);
			showimg(mergedHUE, "Flow Merged Hue relative", false, quantileBot, quantileTop);
		}
		if (1)//BINARY sum (relative)
		{
			Mat mergedBIN = combineTwoPics(medFlowXrelative, FlowPic, BINARYBLUERED, sigma);
			showimg(mergedBIN, "Flow Merged Binary relative", false, quantileBot, quantileTop);
		}
		if (1)//phase and magnitude
		{
			Mat magn, phas;
			magnitude(medFlowXrelative, medFlowY, magn);
			phase(medFlowXrelative, medFlowY, phas);
			showimg(magn, "Flow Magnitude relative", true, quantileBot, quantileTop);
			showimg(phas, "Flow Phase relative", true, quantileBot, quantileTop);
		}
	}
};

double absoluteSubpixelRegistrationError(IPCsettings& set, const Mat& src, double noisestddev, double maxShift, double accuracy);

double IPCparOptFun(std::vector<double>& args, const IPCsettings& settingsMaster, const Mat& source, double noisestddev, double maxShift, double accuracy);

void optimizeIPCParameters(const IPCsettings& settingsMaster, std::string pathInput, std::string pathOutput, double maxShift, double accuracy, unsigned runs, Logger* logger, AbstractPlot1D* plt);

void optimizeIPCParametersForAllWavelengths(const IPCsettings& settingsMaster, double maxShift, double accuracy, unsigned runs, Logger* logger);

DiffrotResults calculateDiffrotProfile(const IPCsettings& IPC_settings, FITStime& FITS_time, int itersPic, int itersX, int itersY, int medianiters, int strajdPic, int deltaPic, int verticalFov, int deltasec, string pathMasterOut, Logger* logger, AbstractPlot1D* plot);

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateLinearSwindFlow(const IPCsettings& set, std::string path, double SwindCropFocusX, double SwindCropFocusY);

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateConstantSwindFlow(const IPCsettings& set, std::string path, double SwindCropFocusX, double SwindCropFocusY);