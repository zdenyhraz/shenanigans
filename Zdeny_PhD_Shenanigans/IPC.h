#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"
#include "fourier.h"
#include "FITS.h"
#include "filtering.h"

using namespace std;
using namespace cv;

class IPCsettings
{
	double stdevLmultiplier = 1.2;
	double stdevHmultiplier = 13;
	int rows = 0;
	int cols = 0;
public:
	double L2size = 11;
	double L1ratio = 0.35;
	int UC = 31;
	double epsilon = 0;
	bool interpolate = 1;
	bool applyWindow = 1;
	bool applyBandpass = 1;
	bool subpixel = 1;
	bool crossCorrel = 0;
	bool normInput = 0;
	bool iterate = 1;
	bool IPCshow = false;
	bool IPCspeak = false;
	double minimalShift = 0;
	Mat bandpass;
	Mat window;

	IPCsettings(int rows, int cols, double stdevLmultiplier, double stdevHmultiplier) : rows(rows), cols(cols), stdevLmultiplier(stdevLmultiplier), stdevHmultiplier(stdevHmultiplier)
	{
		bandpass = bandpassian(rows, cols, stdevLmultiplier, stdevHmultiplier);
		window = edgemask(rows, cols);
	}

	void setBandpassParameters(double StdevLmultiplier, double StdevHmultiplier)
	{
		stdevLmultiplier = StdevLmultiplier;
		stdevHmultiplier = StdevHmultiplier;
		bandpass = bandpassian(rows, cols, stdevLmultiplier, stdevHmultiplier);
	}

	void setSize(int Rows, int Cols)
	{
		rows = Rows;
		cols = Cols;
		window = edgemask(rows, cols);
	}

	int getrows() const
	{
		return rows;
	}

	int getcols() const
	{
		return cols;
	}
};

Point2d phasecorrel(const Mat& sourceimg1In, const Mat& sourceimg2In, IPCsettings& IPC_set, double* corrQuality = nullptr);

void alignPics(const Mat& input1, const Mat& input2, Mat &output, IPCsettings IPC_set);

Mat AlignStereovision(const Mat& img1In, const Mat& img2In);

void alignPicsDebug(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings);

void registrationDuelDebug(IPCsettings& IPC_settings1, IPCsettings& IPC_settings2);

std::tuple<Mat, Mat> calculateFlowMap(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings, double qualityRatio);
