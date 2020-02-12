#include "stdafx.h"
#include "IPC.h"

Mat AlignStereovision(const Mat& img1In, const Mat& img2In)
{
	Mat img1 = img1In.clone();
	Mat img2 = img2In.clone();
	normalize(img1, img1, 0, 65535, CV_MINMAX);
	normalize(img2, img2, 0, 65535, CV_MINMAX);
	img1.convertTo(img1, CV_16U);
	img2.convertTo(img2, CV_16U);

	double d = 0.0;
	double g = 0.5;

	vector<double> rgb1 = { 1. - d, g, d };
	vector<double> rgb2 = { d, 1. - g, 1. - d };

	vector<Mat> channels1(3);
	vector<Mat> channels2(3);
	Mat img1CLR, img2CLR;

	channels1[0] = rgb1[2] * img1;
	channels1[1] = rgb1[1] * img1;
	channels1[2] = rgb1[0] * img1;

	channels2[0] = rgb2[2] * img2;
	channels2[1] = rgb2[1] * img2;
	channels2[2] = rgb2[0] * img2;

	merge(channels1, img1CLR);
	merge(channels2, img2CLR);

	Mat result = img1CLR + img2CLR;
	normalize(result, result, 0, 65535, CV_MINMAX);

	return result;
}

void alignPicsDebug(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings)
{
	/*
	FitsParams params;
	Mat img1 = loadfits("D:\\MainOutput\\304A.fits", params, fitsType::AIA);
	Mat img2 = loadfits("D:\\MainOutput\\171A.fits", params, fitsType::AIA);
	*/

	Mat img1 = img1In.clone();
	Mat img2 = img2In.clone();

	filterSettings filterSet1(10, 4500, 1);
	filterSettings filterSet2(10, 0, 1);
	filterSettings filterSetASV(1, 0, 1);

	img1 = filterContrastBrightness(img1, filterSet1.contrast, filterSet1.brightness);
	img2 = filterContrastBrightness(img2, filterSet2.contrast, filterSet2.brightness);

	img1 = gammaCorrect(img1, filterSet1.gamma);
	img2 = gammaCorrect(img2, filterSet2.gamma);

	Point2f center((float)img1.cols / 2, (float)img1.rows / 2);
	bool artificial = true;
	if (artificial)
	{
		//artificial transform for testing
		//img2 = img1.clone();
		double angle = 70;//70;
		double scale = 1.2;//1.2;
		double shiftX = -950;//-958;
		double shiftY = 1050;//1050;

		cout << "Artificial parameters:" << endl << "Angle: " << angle << endl << "Scale: " << scale << endl << "ShiftX: " << shiftX << endl << "ShiftY: " << shiftY << endl;
		Mat R = getRotationMatrix2D(center, angle, scale);
		Mat T = (Mat_<float>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
		warpAffine(img2, img2, T, cv::Size(img1.cols, img1.rows));
		warpAffine(img2, img2, R, cv::Size(img1.cols, img1.rows));
	}

	showimg(img1, "src1");
	showimg(img2, "src2");
	if (0) saveimg("D:\\MainOutput\\align\\src1.png", img1);
	if (0) saveimg("D:\\MainOutput\\align\\src2.png", img2);

	//show misaligned pics
	showimg(gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma), "not aligned_stereo");
	if (0) saveimg("D:\\MainOutput\\align\\ASV_notAligned.png", gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma));

	//now align them
	alignPics(img1, img2, img2, IPC_settings);

	//show aligned pics
	showimg(gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma), "aligned_stereo");
	if (0) saveimg("D:\\MainOutput\\align\\ASV_Aligned.png", gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma));

	cout << "Finito senor" << endl;
}

void registrationDuelDebug(IPCsettings& IPC_settings1, IPCsettings& IPC_settings2)
{
	if (IPC_settings1.getcols() == 0 || IPC_settings2.getcols() == 0)
	{
		cout << "> please initialize IPC_settings1 and IPC_settings2 to run benchmarks" << endl;
		return;
	}
	//initialize main parameters
	double artificialShiftRange = IPC_settings1.getcols() / 8;
	int trialsPerStartPos = 1000;
	vector<double> startFractionX = { 0.5 };
	vector<double> startFractionY = { 0.5 };
	int progress = 0;

	//load the test picture
	FitsParams params;
	Mat src1 = loadfits("D:\\MainOutput\\HMI.fits", params);
	//Mat src1 = imread("D:\\MainOutput\\png\\HMI_proc.png", IMREAD_ANYDEPTH);
	Mat edgemaska = edgemask(IPC_settings1.getcols(), IPC_settings1.getcols());

	//initialize .csv output
	std::ofstream listing("D:\\MainOutput\\CSVs\\tempBenchmarks.csv", std::ios::out | std::ios::trunc);
	listing << IPC_settings1.getcols() << endl;

	#pragma omp parallel for
	for (int startPos = 0; startPos < startFractionX.size(); startPos++)
	{
		//testing shifts @different picture positions
		int startX = src1.cols * startFractionX[startPos];
		int startY = src1.rows * startFractionY[startPos];
		#pragma omp parallel for
		for (int trial = 0; trial < trialsPerStartPos; trial++)
		{
			//shift entire src2 pic
			Mat src2;
			double shiftX = (double)trial / trialsPerStartPos * artificialShiftRange;
			double shiftY = 0;
			Mat T = (Mat_<float>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
			warpAffine(src1, src2, T, cv::Size(src1.cols, src1.rows));

			//crop both pics
			Mat crop1 = roicrop(src1, startX, startY, IPC_settings1.getcols(), IPC_settings1.getcols());
			Mat crop2 = roicrop(src2, startX, startY, IPC_settings2.getcols(), IPC_settings2.getcols());

			//calculate shifts
			Point2d shifts1 = phasecorrel(crop1, crop2, IPC_settings1);
			Point2d shifts2 = phasecorrel(crop1, crop2, IPC_settings2);

			//list results to .csv
			#pragma omp critical
			{
				listing << shiftX << delimiter << shifts1.x << delimiter << shifts2.x << endl;
				progress++;
				cout << "> IPC benchmark progress " << progress << " / " << trialsPerStartPos * startFractionX.size() << endl;
			}
		}
	}
}

std::tuple<Mat, Mat> calculateFlowMap(const Mat& img1In, const Mat& img2In, IPCsettings& IPC_settings, double qualityRatio)
{
	Mat img1 = img1In.clone();
	Mat img2 = img2In.clone();

	int rows = qualityRatio * img1.rows;
	int cols = qualityRatio * img1.cols;
	int win = IPC_settings.getcols();

	Mat flowX = Mat::zeros(rows, cols, CV_64F);
	Mat flowY = Mat::zeros(rows, cols, CV_64F);

	int pad = win / 2 + 1;
	volatile int progress = 0;

	#pragma omp parallel for
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			int r_ = (double)r / qualityRatio;
			int c_ = (double)c / qualityRatio;

			if (c_ < pad || r_ < pad || c_ > (img1.cols - pad) || r_ > (img1.rows - pad))
			{
				flowX.at<double>(r, c) = 0;
				flowY.at<double>(r, c) = 0;
			}
			else
			{
				Mat crop1 = roicrop(img1, c_, r_, win, win);
				Mat crop2 = roicrop(img2, c_, r_, win, win);
				auto shift = phasecorrel(crop1, crop2, IPC_settings);
				flowX.at<double>(r, c) = shift.x;
				flowY.at<double>(r, c) = shift.y;
			}		
		}

		#pragma omp critical
		{
			progress++;
			cout << "> calculating flow map, " << (double)progress / rows * 100 << "% done." << endl;
		}
	}
	return std::make_tuple(flowX, flowY);
}