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
	int Cwin;
	double L2size;
	double L1ratio;
	int UC;
	double stdevLmultiplier;
	double stdevHmultiplier;
	double epsilon;
	bool interpolate;
	bool window;
	bool bandpass;
	bool subpixel;
	bool crossCorrel;
	bool normInput;
	bool iterate;

	bool IPCshow = false;
	bool IPCspeak = false;
	double minimalShift = 0;

	IPCsettings() : Cwin(0) {}
};

Point2d phasecorrel(Mat& sourceimg1In, Mat& sourceimg2In, IPCsettings& IPC_set, Mat& windowMat, Mat& bandpassMat, double* corrQuality = nullptr);

void alignPics(Mat& input1, Mat& input2, Mat &output, IPCsettings IPC_set, bool calcShift, bool calcRotScale);

Mat AlignStereovision(Mat& img1In, Mat& img2In);

void alignPicsDebug(Mat& img1In, Mat& img2In, IPCsettings& IPC_settings);

void registrationDuelDebug(IPCsettings& IPC_settings1, IPCsettings& IPC_settings2);

std::tuple<Mat, Mat> calculateFlowMap(Mat& img1In, Mat& img2In, IPCsettings& IPC_settings, double qualityRatio);