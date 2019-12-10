
#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"
#include "FITS.h"
#include "IPC.h"
#include "optimization.h"
#include "logger.h"

using namespace std;
using namespace cv;

static std::vector<string> WAVELENGTHS_STR = { "HMI" };
static std::vector<double> STDDEVS(WAVELENGTHS_STR.size(), 0);

struct GUIsettings
{
	int PCWINDOW;
	double PCroiL2Size;
	double PCroiL1SizeRatio;
	int delta_pic;
	int upsamplecoeff;
	double stdevLmult;
	double stdevHmult;
	double snrshit;
	bool usePredicted_shift;
	bool speakYN;
	bool showYN;
	bool writeYN;
	bool useInterp;
	bool useIterate;
	bool useBandpass;
	bool useWindow;
	bool useSubpixel;
	int iters_horizontal;
	int iters_vertical;
	int iters_pic;
	int stride_pic;
	bool useCrossCorrel;
	bool useNorminput;
	bool alternate;
	bool colorr;
	double quanBot;
	double quanTop;
	int medSize;
	double mergeAlp;
	int vertical_fov;
	double sigmaa;
	double contrastt;
	double brightnesss;
	double gamaa;
	int iters_median;
	bool twoCorrels;
	std::string diskk;
	std::string pathMasterIn;
	std::string pathMasterOut;
	std::string urlstart;
};

std::vector<double> diffrotProfileAverage(Mat& flow, int colS = 0);

struct FlowResults
{
	Mat FlowPic;
	Mat FlowX;
	Mat FlowY;
	Size2i exportSize = Size2i(0, 0);

	void showResults(double quantileBot, double quantileTop, int medianSize, double mergeAlpha, double degree)
	{
		//heatmaps (median blurred + quantiled)
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
		std::vector<double> profileAverage = diffrotProfileAverage(medFlowX, medFlowX.cols / 20);
		std::vector<double> profileAverageSmooth = polyfit(profileAverage, 4);
		Mat profileAverageSmoothM = matFromVector(profileAverageSmooth, medFlowX.cols);
		showimg(profileAverageSmoothM, "Flow AvgSmooth (Med+Quan)", true, quantileBot, quantileTop, exportSize);
		Mat medFlowXrelative = medFlowX - profileAverageSmoothM;
		if (1)//source
		{
			showimg(FlowPic, "Flow Source", false, 0, 1, exportSize);
		}
		if (1)//flow
		{
			showimg(medFlowX, "Flow X", true, quantileBot, quantileTop, exportSize);
			showimg(medFlowY, "Flow Y", true, quantileBot, quantileTop, exportSize);
		}
		if (1)//relative flow
		{
			Mat medFlowXrelative = medFlowX - profileAverageSmoothM;
			showimg(medFlowXrelative, "Flow relative", true, quantileBot, quantileTop, exportSize);
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
			Mat mergedGHE = (1. - mergeAlpha)*FlowPicBGR + mergeAlpha * medFlowX_BGR;
			showimg(mergedGHE, "Flow MergedGHE", false, quantileBot, quantileTop, exportSize);
		}
		if (1)//HUE sum
		{
			Mat mergedHUE = combineTwoPics(applyColorMapZdeny(medFlowX, quantileBot, quantileTop, false), FlowPic, HUEBRIGHT);
			showimg(mergedHUE, "Flow MergedHUE", false, quantileBot, quantileTop, exportSize);
		}
		if (1)//HUE sum relative
		{
			Mat mergedHUE = combineTwoPics(applyColorMapZdeny(medFlowXrelative, quantileBot, quantileTop, false), FlowPic, HUEBRIGHT);
			showimg(mergedHUE, "Flow MergedHUE relative", false, quantileBot, quantileTop, exportSize);
		}
		if (1)//BINARY sum (relative)
		{
			Mat mergedBIN = combineTwoPics(medFlowXrelative, FlowPic, BINARYBLUERED, degree);
			showimg(mergedBIN, "Flow MergedRelativeBIN", false, quantileBot, quantileTop, exportSize);
		}
		if (1)//phase and magnitude
		{
			Mat magn, phas;
			magnitude(medFlowXrelative, medFlowY, magn);
			phase(medFlowXrelative, medFlowY, phas);
			showimg(magn, "Flow Magn relative", false, quantileBot, quantileTop, exportSize);
			showimg(phas, "Flow phase relative", false, quantileBot, quantileTop, exportSize);
		}
	}

	void saveResults(double quantileBot, double quantileTop, int medianSize, double mergeAlpha)
	{

	}
};

void makeOutputDirs(std::string path);

double absoluteSubpixelRegistrationError(IPCsettings& IPC_set, Mat& src, double noisestddev, double maxShiftRatio, double accuracy);

double IPCparOptFun(std::vector<double>& args, const IPCsettings& settingsMaster, Mat& source, double noisestddev, double maxShiftRatio, double accuracy);

void optimizeIPCParameters(const IPCsettings& settingsMaster, std::string pathInput, std::string pathOutput, double maxShiftRatio, double accuracy, unsigned runs, Logger* logger);

void optimizeIPCParametersForAllWavelengths(const IPCsettings& settingsMaster, const GUIsettings& guiset, double maxShiftRatio, double accuracy);

void calculateDiffrotProfile(IPCsettings& IPC_settings, IPCsettings& IPC_settings1, IPCsettings& IPC_settings2, FITStime& FITS_time, FlowResults* MainResults, bool twoCorrels, int iters, int itersX, int itersY, int medianiters, int strajdPic, int deltaPic, int verticalFov, int pcwindowsize, int deltasec, string pathMasterOut);