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
	std::vector<double> FlowYfit;
	std::vector<double> FlowXpred;
	std::vector<std::vector<double>> FlowXfits;
	std::vector<std::vector<double>> FlowYfits;

	void showsaveResults(double quantileBot, double quantileTop, int medianSize, double sigma, std::string dirpath)
	{
		dirpath += "pics//";
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
		Mat medFlowXrelativeF = medFlowX - matFromVector(FlowXfit, medFlowX.cols);
		Mat medFlowXrelativeFF = medFlowX - matFromVectorC(FlowXfits);
		Mat medFlowXrelativeP = medFlowX - matFromVector(FlowXpred, medFlowX.cols);
		showimg(matFromVector(FlowXfit, medFlowX.cols), "FlowX Fit", true);
		showimg(matFromVectorC(FlowXfits), "FlowX FFit", true);
		showimg(matFromVector(FlowXpred, medFlowX.cols), "FlowX Pred", true);

		//now show various stuff
		if (1)//source
		{
			showimg(FlowPic, "Flow Source");
			saveimg(dirpath + "FlowPic.png", FlowPic, true);
		}
		if (1)//flow
		{
			showimg(medFlowX, "Flow X", true, quantileBot, quantileTop);
			showimg(medFlowY, "Flow Y", true, quantileBot, quantileTop);
			saveimg(dirpath + "FlowX.png", applyColorMapZdeny(medFlowX, quantileBot, quantileTop, true), true);
			saveimg(dirpath + "FlowY.png", applyColorMapZdeny(medFlowY, quantileBot, quantileTop, true), true);
		}
		if (1)//relative flow
		{
			showimg(medFlowXrelativeF, "FlowX relative Fit", true, quantileBot, quantileTop);
			showimg(medFlowXrelativeFF, "FlowX relative FFit", true, quantileBot, quantileTop);
			showimg(medFlowXrelativeP, "FlowX relative Pred", true, quantileBot, quantileTop);
			saveimg(dirpath + "FlowXRelFit.png", applyColorMapZdeny(medFlowXrelativeF, quantileBot, quantileTop, true), true);
			saveimg(dirpath + "FlowXRelFFit.png", applyColorMapZdeny(medFlowXrelativeFF, quantileBot, quantileTop, true), true);
			saveimg(dirpath + "FlowXRelPred.png", applyColorMapZdeny(medFlowXrelativeP, quantileBot, quantileTop, true), true);
		}
		if (0)//ghetto sum
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
			showimg(mergedGHE, "FlowX Merged Ghetto", false, quantileBot, quantileTop);
		}
		if (0)//HUE sum
		{
			Mat mergedHUE = combineTwoPics(applyColorMapZdeny(medFlowX, quantileBot, quantileTop, false), FlowPic, HUEBRIGHT);
			showimg(mergedHUE, "FlowX Merged Hue", false, quantileBot, quantileTop);
		}
		if (0)//HUE sum relative
		{
			Mat mergedHUE = combineTwoPics(medFlowXrelativeP, FlowPic, HUEBRIGHT);
			showimg(mergedHUE, "FlowX Merged Hue relative Pred", false, quantileBot, quantileTop);
		}
		if (1)//BINARY sum relative
		{
			Mat mergedBINF = combineTwoPics(medFlowXrelativeF, FlowPic, BINARYBLUERED, sigma);
			Mat mergedBINFF = combineTwoPics(medFlowXrelativeFF, FlowPic, BINARYBLUERED, sigma);
			Mat mergedBINP = combineTwoPics(medFlowXrelativeP, FlowPic, BINARYBLUERED, sigma);
			showimg(mergedBINF, "FlowX Merged Binary relative Fit", false, quantileBot, quantileTop);
			showimg(mergedBINFF, "FlowX Merged Binary relative FFit", false, quantileBot, quantileTop);
			showimg(mergedBINP, "FlowX Merged Binary relative Pred", false, quantileBot, quantileTop);
			saveimg(dirpath + "FlowXBinFit.png", mergedBINF, true);
			saveimg(dirpath + "FlowXBinFFit.png", mergedBINFF, true);
			saveimg(dirpath + "FlowXBinPred.png", mergedBINP, true);
		}
		if (1)//phase and magnitude
		{
			//absolute
			Mat magn, phas;
			magnitude(medFlowX, medFlowY, magn);
			phase(medFlowX, medFlowY, phas);
			showimg(magn, "FlowX Magnitude", true, quantileBot, quantileTop);
			showimg(phas, "FlowX Phase", true, quantileBot, quantileTop);
			saveimg(dirpath + "FlowXMagn.png", applyColorMapZdeny(magn, quantileBot, quantileTop, true), true);
			saveimg(dirpath + "FlowXPhas.png", applyColorMapZdeny(phas, quantileBot, quantileTop, true), true);

			//relative
			Mat magnP, phasP;
			magnitude(medFlowXrelativeP, medFlowY, magnP);
			phase(medFlowXrelativeP, medFlowY, phasP);
			showimg(magnP, "FlowX Magnitude relative Pred", true, quantileBot, quantileTop);
			showimg(phasP, "FlowX Phase relative Pred", true, quantileBot, quantileTop);
			saveimg(dirpath + "FlowXRelPredMagn.png", applyColorMapZdeny(magnP, quantileBot, quantileTop, true), true);
			saveimg(dirpath + "FlowXRelPredPhas.png", applyColorMapZdeny(phasP, quantileBot, quantileTop, true), true);
		}
	}
};

double absoluteSubpixelRegistrationError(IPCsettings& set, const Mat& src, double noisestddev, double maxShift, double accuracy);

double IPCparOptFun(std::vector<double>& args, const IPCsettings& settingsMaster, const Mat& source, double noisestddev, double maxShift, double accuracy);

void optimizeIPCParameters(const IPCsettings& settingsMaster, std::string pathInput, std::string pathOutput, double maxShift, double accuracy, unsigned runs, Logger* logger, AbstractPlot1D* plt);

void optimizeIPCParametersForAllWavelengths(const IPCsettings& settingsMaster, double maxShift, double accuracy, unsigned runs, Logger* logger);

DiffrotResults calculateDiffrotProfile(const IPCsettings& IPC_settings, FITStime& FITS_time, int itersPic, int itersX, int itersY, int medianiters, int strajdPic, int deltaPic, int verticalFov, int deltasec, Logger* logger, AbstractPlot1D* pltX, AbstractPlot1D* pltY);

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateLinearSwindFlow(const IPCsettings& set, std::string path, double SwindCropFocusX, double SwindCropFocusY);

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateConstantSwindFlow(const IPCsettings& set, std::string path, double SwindCropFocusX, double SwindCropFocusY);

double DiffrotMerritFunction(const IPCsettings& set, const std::vector<std::pair<Mat, Mat>>& pics, const std::vector<std::pair<fitsParams, fitsParams>>& params, int itersX, int itersY, int itersMedian, int strajdPic, int deltaPic, int verticalFov, int deltaSec, AbstractPlot1D* pltX = nullptr);

double DiffrotMerritFunctionWrapper(std::vector<double>& arg, const std::vector<std::pair<Mat, Mat>>& pics, const std::vector<std::pair<fitsParams, fitsParams>>& params, int itersX, int itersY, int itersMedian, int strajdPic, int deltaPic, int verticalFov, int deltaSec, AbstractPlot1D* pltX = nullptr);