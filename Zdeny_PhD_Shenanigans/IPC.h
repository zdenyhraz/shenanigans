#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"
#include "fourier.h"
#include "FITS.h"
#include "filtering.h"
#include "logger.h"
#include "plotter.h"

using namespace std;
using namespace cv;

class IPCsettings
{
private:
	double stdevLmultiplier = 5;
	double stdevHmultiplier = 100;
	int rows = 0;
	int cols = 0;
public:
	double L2size = 17;
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
	double minimalShift = 0;
	Mat bandpass;
	Mat window;

	IPCsettings(int Rows, int Cols, double StdevLmultiplier, double StdevHmultiplier) : rows(Rows), cols(Cols), stdevLmultiplier(StdevLmultiplier), stdevHmultiplier(StdevHmultiplier)
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
		bandpass = bandpassian(rows, cols, stdevLmultiplier, stdevHmultiplier);
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

	double getL() const
	{
		return stdevLmultiplier;
	}

	double getH() const
	{
		return stdevHmultiplier;
	}
};

inline Point2d phasecorrel(const Mat& sourceimg1In, const Mat& sourceimg2In, const IPCsettings& set, Logger* logger = nullptr, AbstractPlot2D* plt = nullptr)
{
	Mat sourceimg1 = sourceimg1In.clone();
	Mat sourceimg2 = sourceimg2In.clone();
	Point2d output;
	Mat L3;
	sourceimg1.convertTo(sourceimg1, CV_64F);
	sourceimg2.convertTo(sourceimg2, CV_64F);
	if (set.normInput)
	{
		normalize(sourceimg1, sourceimg1, 0, 1, CV_MINMAX);
		normalize(sourceimg2, sourceimg2, 0, 1, CV_MINMAX);
	}
	if (set.applyWindow)
	{
		multiply(sourceimg1, set.window, sourceimg1);
		multiply(sourceimg2, set.window, sourceimg2);
	}
	if (set.IPCshow)
	{
		showimg(sourceimg1, "IPC src1");
		showimg(sourceimg2, "IPC src2");
		showimg(set.bandpass, "IPC bandpass");
		showimg(set.window, "IPC window");
	}
	Mat DFT1 = fourier(sourceimg1);
	Mat DFT2 = fourier(sourceimg2);
	Mat planes1[2];
	Mat planes2[2];
	Mat CrossPowerPlanes[2];
	split(DFT1, planes1);
	split(DFT2, planes2);
	planes1[1] = planes1[1].mul(-1.0);//complex conjugate of second pic
	CrossPowerPlanes[0] = planes1[0].mul(planes2[0]) - planes1[1].mul(planes2[1]);//pointwise multiplications real
	CrossPowerPlanes[1] = planes1[0].mul(planes2[1]) + planes1[1].mul(planes2[0]);//imag
	if (!set.crossCorrel)
	{
		Mat magnre, magnim;
		pow(CrossPowerPlanes[0], 2, magnre);
		pow(CrossPowerPlanes[1], 2, magnim);
		Mat normalizationdenominator = magnre + magnim;
		sqrt(normalizationdenominator, normalizationdenominator);
		CrossPowerPlanes[0] /= (normalizationdenominator + set.epsilon);
		CrossPowerPlanes[1] /= (normalizationdenominator + set.epsilon);
	}
	if (logger) logger->Log("cross-power spectrum calculated", INFO);
	Mat CrossPower;
	merge(CrossPowerPlanes, 2, CrossPower);
	if (set.IPCshow) { showfourier(CrossPower, false, true, "crosspowerFFTmagn", "crosspowerFFTphase"); }
	if (set.applyBandpass)
	{
		CrossPower = bandpass(CrossPower, set.bandpass);
		if (logger) logger->Log("cross-power spectrum bandpassed", INFO);
	}
	if (1)//CORRECT - complex magnitude - input can be real or complex whatever
	{
		Mat L3complex;
		dft(CrossPower, L3complex, DFT_INVERSE + DFT_SCALE);
		Mat L3planes[2];
		split(L3complex, L3planes);
		if (0)
		{
			auto minmaxReal = minMaxMat(L3planes[0]);
			auto minmaxImag = minMaxMat(L3planes[1]);
			if (logger) logger->Log("L3 real min/max: " + to_string(std::get<0>(minmaxReal)) + " / " + to_string(std::get<1>(minmaxReal)), INFO);
			if (logger) logger->Log("L3 imag min/max: " + to_string(std::get<0>(minmaxImag)) + " / " + to_string(std::get<1>(minmaxImag)), INFO);
		}
		magnitude(L3planes[0], L3planes[1], L3);
		Mat L3phase;
		phase(L3planes[0], L3planes[1], L3phase);
		if (set.IPCshow) { Mat L3pv; resize(L3phase, L3pv, cv::Size(2000, 2000), 0, 0, INTER_NEAREST); showimg(L3pv, "L3phase", true); }
	}
	else//real only (assume pure real input)
	{
		dft(CrossPower, L3, DFT_INVERSE + DFT_SCALE + DFT_REAL_OUTPUT);
	}
	L3 = quadrantswap(L3);
	//L3 = L3.mul(quadrantMask(L3.rows, L3.cols));//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< this is jsut s-wind (HARDCODED)
	if (logger) logger->Log("inverse fourier of cross-power spectrum calculated", INFO);
	if (set.minimalShift) L3 = L3.mul(1 - kirkl(L3.rows, L3.cols, set.minimalShift));
	Point2i L3peak, L3bot;
	double maxR, minR;
	minMaxLoc(L3, &minR, &maxR, &L3bot, &L3peak);
	Point2d L3mid(L3.cols / 2, L3.rows / 2);
	Point2d imageshift_PIXEL(L3peak.x - L3mid.x, L3peak.y - L3mid.y);
	if (set.IPCshow)
	{
		Mat L3v;
		resize(L3, L3v, cv::Size(2000, 2000), 0, 0, INTER_NEAREST);
		showimg(crosshair(L3v, Point2d(round((double)(L3peak.x) * 2000. / (double)L3.cols), round((double)(L3peak.y) * 2000. / (double)L3.rows))), "L3", true, 0, 1);
		Mat L3vl; resize(L3, L3vl, cv::Size(2000, 2000), 0, 0, INTER_NEAREST);
		for (int j = 0; j < 10; j++)
		{
			L3vl += Scalar::all(1);
			log(L3vl, L3vl);
			normalize(L3vl, L3vl, 0, 1, CV_MINMAX);
		}
		showimg(crosshair(L3vl, Point2d(round((double)(L3peak.x) * 2000. / (double)L3.cols), round((double)(L3peak.y) * 2000. / (double)L3.rows))), "L3 log", true, 0, 1);
	}
	if (logger)
	{
		logger->Log("Phase correlation max: " + to_string(maxR) + " at location: " + to_string(L3peak), INFO);
		logger->Log("Calculated shift with pixel accuracy: " + to_string(imageshift_PIXEL) + " pixels", INFO);
		logger->Log("phase correlation calculated with pixel accuracy", SUBEVENT);
	}
	output = imageshift_PIXEL;

	if (set.subpixel)
	{
		//first check for degenerate peaklocs
		int L2size = set.L2size;
		if (!(L2size % 2)) L2size++;//odd!+
		if (((L3peak.x - L2size / 2) < 0) || ((L3peak.y - L2size / 2) < 0) || ((L3peak.x + L2size / 2 + 1) > sourceimg1.cols) || ((L3peak.y + L2size / 2 + 1) > sourceimg1.rows))
		{
			if (logger) logger->Log("degenerate peak, results might be inaccurate!", FATAL);
		}
		else
		{
			Point2d imageshift_SUBPIXEL;
			Mat L2 = roicrop(L3, L3peak.x, L3peak.y, L2size, L2size);
			Point2d L2mid(L2.cols / 2, L2.rows / 2);
			Mat L2U;
			if (set.interpolate)
				resize(L2, L2U, L2.size()*set.UC, 0, 0, INTER_LINEAR);
			else
				resize(L2, L2U, L2.size()*set.UC, 0, 0, INTER_NEAREST);
			Point2d L2Umid(L2U.cols / 2, L2U.rows / 2);
			if (set.IPCshow) { Mat L2v; resize(L2, L2v, cv::Size(2000, 2000), 0, 0, INTER_NEAREST); showimg(crosshair(L2v, L2mid * 2000 / L2.cols), "L2", true); }
			if (set.IPCshow) { Mat L2Uv; resize(L2U, L2Uv, cv::Size(2000, 2000), 0, 0, INTER_LINEAR); showimg(crosshair(L2Uv, L2Umid * 2000 / L2U.cols), "L2U", true, 0, 1); }
			Point2d L2Upeak(L2U.cols / 2, L2U.rows / 2);
			if (logger)
			{
				logger->Log("L2Upeak location before iterations: " + to_string(L2Upeak), INFO);
				logger->Log("L2Upeak location before iterations findCentroid double: " + to_string(findCentroidDouble(L2U)), INFO);
			}
			int L1size = std::round((double)L2U.cols*set.L1ratio);
			if (!(L1size % 2)) L1size++;//odd!+
			Mat L1;
			Point2d L1mid;
			if (!set.iterate)
			{
				L1 = roicrop(L3, L3peak.x, L3peak.y, 5, 5);
				L1mid = Point2d(L1.cols / 2, L1.rows / 2);
				imageshift_SUBPIXEL = (Point2d)L3peak - L3mid + findCentroidDouble(L1) - L1mid;
				if (set.IPCshow) { Mat L1v; resize(L1, L1v, cv::Size(2000, 2000), 0, 0, INTER_NEAREST); showimg(crosshair(L1v, findCentroidDouble(L1) * 2000 / L1.cols), "L1 a4r", true); }
			}
			else
			{
				L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
				L1mid = Point2d(L1.cols / 2, L1.rows / 2);
				if (set.IPCshow) { Mat L1v; resize(L1, L1v, cv::Size(2000, 2000), 0, 0, INTER_LINEAR); showimg(crosshair(L1v, L1mid * 2000 / L1.cols), "L1 be4", true); }
				int maxPCit = 50;
				for (int i = 0; i < maxPCit; i++)
				{
					if (logger) logger->Log("======= iteration " + to_string(i) + " =======", SUBEVENT);
					L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
					Point2d L1peak = findCentroidDouble(L1);
					if (logger) logger->Log("L1peak: " + to_string(L1peak), INFO);
					L2Upeak.x += round(L1peak.x - L1mid.x);
					L2Upeak.y += round(L1peak.y - L1mid.y);
					if ((L2Upeak.x > (L2U.cols - L1mid.x - 1)) || (L2Upeak.y > (L2U.rows - L1mid.y - 1)) || (L2Upeak.x < (L1mid.x + 1)) || (L2Upeak.y < (L1mid.y + 1)))
					{
						if (logger) logger->Log("breaking out of IPC iterations, centroid diverges", FATAL);
						break;
					}
					if (logger) logger->Log("L1peak findCentroid double delta: " + to_string(findCentroidDouble(L1).x - L1mid.x) + " , " + to_string(findCentroidDouble(L1).y - L1mid.y), INFO);
					if (logger) logger->Log("Resulting L2Upeak in this iteration: " + to_string(L2Upeak), INFO);
					if ((abs(L1peak.x - L1mid.x) < 0.5) && (abs(L1peak.y - L1mid.y) < 0.5))
					{
						L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
						if (logger) logger->Log("Iterative PC accuracy reached", SUBEVENT);
						if (logger) logger->Log("L2Upeak: " + to_string(L2Upeak), INFO);
						if (set.IPCshow)
						{
							Mat L1v;
							resize(L1, L1v, cv::Size(2000, 2000), 0, 0, INTER_LINEAR);
							showimg(crosshair(L1v, L1mid * 2000 / L1.cols), "L1 a4r", true);
							if (plt) plt->plot(matToVect2(L1v));
						}
						break;
					}
					if (i == (maxPCit - 1))
					{
						if (logger) logger->Log("IPC out of iterations - diverges", FATAL);
					}
				}
				imageshift_SUBPIXEL.x = (double)L3peak.x - (double)L3mid.x + 1.0 / (double)set.UC*((double)L2Upeak.x - (double)L2Umid.x + findCentroidDouble(L1).x - (double)L1mid.x);//image shift in L3 - final
				imageshift_SUBPIXEL.y = (double)L3peak.y - (double)L3mid.y + 1.0 / (double)set.UC*((double)L2Upeak.y - (double)L2Umid.y + findCentroidDouble(L1).y - (double)L1mid.y);//image shift in L3 - final
			}

			if (logger)
			{
				logger->Log("iterative phase correlation calculated with subpixel accuracy", SUBEVENT);
				logger->Log("L3 size: " + to_string(L3.cols) + ", L2 size: " + to_string(L2.cols) + ", L2U size: " + to_string(L2U.cols) + ", L1 size: " + to_string(L1.cols), INFO);
				logger->Log("Upsample pixel accuracy theoretical maximum: " + to_string(1.0 / (double)set.UC), INFO);
				logger->Log("Calculated shift with pixel accuracy: " + to_string(imageshift_PIXEL) + " pixels ", INFO);
				logger->Log("Calculated shift with SUBpixel accuracy1: " + to_string(imageshift_SUBPIXEL) + " pixels ", SUBEVENT);
			}
			output = imageshift_SUBPIXEL;
		}
	}
	return output;
}

inline void alignPics(const Mat& input1, const Mat& input2, Mat &output, IPCsettings set)
{
	Mat estR, estT;
	Point2f center((float)input1.cols / 2, (float)input1.rows / 2);
	output = input2.clone();

	if (1)//calculate rotation and scale
	{
		Mat img1FT = fourier(input1);
		Mat img2FT = fourier(input2);
		img1FT = quadrantswap(img1FT);
		img2FT = quadrantswap(img2FT);
		Mat planes1[2] = { Mat::zeros(img1FT.size(),CV_64F), Mat::zeros(img1FT.size(),CV_64F) };
		Mat planes2[2] = { Mat::zeros(img1FT.size(),CV_64F), Mat::zeros(img1FT.size(),CV_64F) };
		split(img1FT, planes1);
		split(img2FT, planes2);
		Mat img1FTm, img2FTm;
		magnitude(planes1[0], planes1[1], img1FTm);
		magnitude(planes2[0], planes2[1], img2FTm);
		bool logar = true;
		if (logar)
		{
			img1FTm += Scalar::all(1.);
			img2FTm += Scalar::all(1.);
			log(img1FTm, img1FTm);
			log(img2FTm, img2FTm);
		}
		normalize(img1FTm, img1FTm, 0, 1, CV_MINMAX);
		normalize(img2FTm, img2FTm, 0, 1, CV_MINMAX);
		Mat img1LP, img2LP;
		double maxRadius = 1.*min(center.y, center.x);
		int flags = INTER_LINEAR + WARP_FILL_OUTLIERS;
		warpPolar(img1FTm, img1LP, cv::Size(input1.cols, input1.rows), center, maxRadius, flags + WARP_POLAR_LOG);    // semilog Polar
		warpPolar(img2FTm, img2LP, cv::Size(input1.cols, input1.rows), center, maxRadius, flags + WARP_POLAR_LOG);    // semilog Polar
		auto LPshifts = phasecorrel(img1LP, img2LP, set);
		cout << "LPshifts: " << LPshifts << endl;
		double anglePredicted = -LPshifts.y / input1.rows * 360;
		double scalePredicted = exp(LPshifts.x * log(maxRadius) / input1.cols);
		cout << "Evaluated rotation: " << anglePredicted << " deg" << endl;
		cout << "Evaluated scale: " << 1. / scalePredicted << " " << endl;
		estR = getRotationMatrix2D(center, -anglePredicted, scalePredicted);
		warpAffine(output, output, estR, cv::Size(input1.cols, input1.rows));
	}

	if (1)//calculate shift
	{
		auto shifts = phasecorrel(input1, output, set);
		cout << "shifts: " << shifts << endl;
		double shiftXPredicted = shifts.x;
		double shiftYPredicted = shifts.y;
		cout << "Evaluated shiftX: " << shiftXPredicted << " px" << endl;
		cout << "Evaluated shiftY: " << shiftYPredicted << " px" << endl;
		estT = (Mat_<float>(2, 3) << 1., 0., -shiftXPredicted, 0., 1., -shiftYPredicted);
		warpAffine(output, output, estT, cv::Size(input1.cols, input1.rows));
	}
}

Mat AlignStereovision(const Mat& img1In, const Mat& img2In);

void alignPicsDebug(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings);

void registrationDuelDebug(IPCsettings& IPC_settings1, IPCsettings& IPC_settings2);

std::tuple<Mat, Mat> calculateFlowMap(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings, double qualityRatio);
