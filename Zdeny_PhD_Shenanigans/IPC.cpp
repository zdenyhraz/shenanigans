#include "stdafx.h"
#include "IPC.h"

Point2d phasecorrel(const Mat& sourceimg1In, const Mat& sourceimg2In, IPCsettings& set, double* corrQuality)
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
	if (set.IPCspeak) std::cout << "cross-power spectrum calculated" << std::endl;
	Mat CrossPower;
	merge(CrossPowerPlanes, 2, CrossPower);
	if (set.IPCshow) { showfourier(CrossPower, false, true, "crosspowerFFTmagn", "crosspowerFFTphase"); }
	if (set.applyBandpass)
	{
		CrossPower = bandpass(CrossPower, set.bandpass);
		if (set.IPCspeak) std::cout << "cross-power spectrum bandpassed" << std::endl;
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
			std::cout << "L3 real min/max: " << std::get<0>(minmaxReal) << " / " << std::get<1>(minmaxReal) << std::endl;
			std::cout << "L3 imag min/max: " << std::get<0>(minmaxImag) << " / " << std::get<1>(minmaxImag) << std::endl;
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
	if (set.IPCspeak) std::cout << "inverse fourier of cross-power spectrum calculated" << std::endl;
	if (corrQuality)
	{
		double minRQ, maxRQ;
		minMaxLoc(L3, &minRQ, &maxRQ, nullptr, nullptr);
		*corrQuality = maxRQ;
	}
	
	//normalize(L3, L3, 0, 1, CV_MINMAX);//unnecessary?
	if (set.minimalShift) L3 = L3.mul(1 - kirkl(L3.rows, L3.cols, set.minimalShift));
	Point2i L3peak;
	Point2i minRloc;
	double maxR, minR;
	minMaxLoc(L3, &minR, &maxR, &minRloc, &L3peak);
	if (set.IPCspeak) std::cout << "Phase correlation max: " << maxR << " at location: " << L3peak << std::endl;
	Point2d L3mid(L3.cols / 2, L3.rows / 2);
	Point2d imageshift_PIXEL(L3peak.x - L3mid.x, L3peak.y - L3mid.y);
	if (set.IPCspeak) std::cout << "Calculated shiftX with pixel accuracy: " << imageshift_PIXEL << " pixels" << std::endl;
	if (set.IPCshow) { Mat L3v; resize(L3, L3v, cv::Size(2000, 2000), 0, 0, INTER_NEAREST); showimg(crosshair(L3v, Point2d(round((double)(L3peak.x) * 2000. / (double)L3.cols), round((double)(L3peak.y) * 2000. / (double)L3.rows))), "L3", true, 0, 1); }
	if (set.IPCshow) 
	{ 
		Mat L3vl; resize(L3, L3vl, cv::Size(2000, 2000), 0, 0, INTER_NEAREST); 
		for (int j = 0; j < 10; j++)
		{
			L3vl += Scalar::all(1);
			log(L3vl, L3vl);
			normalize(L3vl, L3vl, 0, 1, CV_MINMAX);
		}
		showimg(crosshair(L3vl, Point2d(round((double)(L3peak.x) * 2000. / (double)L3.cols), round((double)(L3peak.y) * 2000. / (double)L3.rows))), "L3 log", true, 0, 1); 
	}
	if (set.IPCspeak) std::cout << "phase correlation calculated with pixel accuracy" << std::endl;
	output = imageshift_PIXEL;

	if (set.subpixel)
	{
		//first check for degenerate peaklocs
		int L2size = set.L2size;
		if (!(L2size % 2)) L2size++;//odd!+
		if (((L3peak.x - L2size / 2) < 0) || ((L3peak.y - L2size / 2) < 0) || ((L3peak.x + L2size / 2 + 1) > sourceimg1.cols) || ((L3peak.y + L2size / 2 + 1) > sourceimg1.rows))
		{
			if (set.IPCspeak) cout << "degenerate peak, results might be inaccurate!" << endl;
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
			if (set.IPCspeak) std::cout << "L2Upeak location before iterations: " << L2Upeak << std::endl;
			if (set.IPCspeak) std::cout << "L2Upeak location before iterations findCentroid double: " << findCentroidDouble(L2U) << std::endl;
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
					if (set.IPCspeak) std::cout << "====================================" << " iterace " << i << " ====================================" << std::endl;
					L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
					Point2d L1peak = findCentroidDouble(L1);
					if (set.IPCspeak) std::cout << "L1peak: " << L1peak << std::endl;
					L2Upeak.x += round(L1peak.x - L1mid.x);
					L2Upeak.y += round(L1peak.y - L1mid.y);
					if ((L2Upeak.x > (L2U.cols - L1mid.x - 1)) || (L2Upeak.y > (L2U.rows - L1mid.y - 1)) || (L2Upeak.x < (L1mid.x + 1)) || (L2Upeak.y < (L1mid.y + 1)))
					{
						if (set.IPCspeak) cout << "breaking out of pc iterations, centroid diverges" << endl;
						break;
					}
					if (set.IPCspeak) std::cout << "L1peak findCentroid double delta: " << findCentroidDouble(L1).x - L1mid.x << " , " << findCentroidDouble(L1).y - L1mid.y << std::endl;
					if (set.IPCspeak) std::cout << "Resulting L2Upeak in this iteration: " << L2Upeak << std::endl;
					if ((abs(L1peak.x - L1mid.x) < 0.5) && (abs(L1peak.y - L1mid.y) < 0.5))
					{
						L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
						if (set.IPCspeak) std::cout << "Iterative PC accuracy reached <<<" << std::endl;
						if (set.IPCspeak) std::cout << "L2Upeak: " << L2Upeak << std::endl;
						if (set.IPCshow) { Mat L1v; resize(L1, L1v, cv::Size(2000, 2000), 0, 0, INTER_LINEAR); showimg(crosshair(L1v, L1mid * 2000 / L1.cols), "L1 a4r", true); }
						if (set.IPCspeak) std::cout << "===================================================================================" << std::endl;
						break;
					}
					if (i == (maxPCit - 1))
					{
						if (set.IPCspeak) cout << "PC out of iterations - diverges" << endl;
					}
				}
				imageshift_SUBPIXEL.x = (double)L3peak.x - (double)L3mid.x + 1.0 / (double)set.UC*((double)L2Upeak.x - (double)L2Umid.x + findCentroidDouble(L1).x - (double)L1mid.x);//image shift in L3 - final
				imageshift_SUBPIXEL.y = (double)L3peak.y - (double)L3mid.y + 1.0 / (double)set.UC*((double)L2Upeak.y - (double)L2Umid.y + findCentroidDouble(L1).y - (double)L1mid.y);//image shift in L3 - final
			}

			if (set.IPCspeak) std::cout << "phase correlation calculated with subpixel accuracy" << std::endl;
			if (set.IPCspeak) std::cout << "Calculated shift with pixel accuracy: " << imageshift_PIXEL << " pixels " << std::endl;
			if (set.IPCspeak) std::cout << "Calculated shift with SUBpixel accuracy1: " << imageshift_SUBPIXEL << " pixels " << std::endl;
			if (set.IPCspeak) std::cout << "L3 size: " << L3.size << ", L2 size: " << L2.size << ", L2U size: " << L2U.size << ", L1 size: " << L1.size << std::endl;
			if (set.IPCspeak) std::cout << "Upsample pixel accuracy theoretical maximum: " << 1.0 / (double)set.UC << std::endl;

			output = imageshift_SUBPIXEL;
		}
	}
	
	return output;
}

void alignPics(const Mat& input1, const Mat& input2, Mat &output, IPCsettings set)
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
	fitsParams params;
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
	fitsParams params;
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