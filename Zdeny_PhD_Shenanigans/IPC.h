#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"
#include "fourier.h"
#include "FITS.h"
#include "filtering.h"

using namespace std;
using namespace cv;

struct IPCsettings
{
	int Cwin = 32;
	double L2size = 11;
	double L1ratio = 0.35;
	int UC = 31;
	double stdevLmultiplier = 1.2;
	double stdevHmultiplier = 13;
	double epsilon = 0;
	bool interpolate = 1;
	bool window = 1;
	bool bandpass = 1;
	bool subpixel = 1;
	bool crossCorrel = 0;
	bool normInput = 0;
	bool iterate = 1;

	bool IPCshow = false;
	bool IPCspeak = false;
	double minimalShift = 0;
};

Point2d phasecorrel(const Mat& sourceimg1In, const Mat& sourceimg2In, IPCsettings& IPC_set, const Mat& windowMat, const Mat& bandpassMat, double* corrQuality = nullptr);

void alignPics(const Mat& input1, const Mat& input2, Mat &output, IPCsettings IPC_set);

Mat AlignStereovision(const Mat& img1In, const Mat& img2In);

void alignPicsDebug(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings);

void registrationDuelDebug(IPCsettings& IPC_settings1, IPCsettings& IPC_settings2);

std::tuple<Mat, Mat> calculateFlowMap(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings, double qualityRatio);
