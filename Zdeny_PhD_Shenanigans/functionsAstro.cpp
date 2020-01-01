//various image processing / optimization / fractal functions
//PhD work of Zdenek Hrazdira
//made during 2018-2019

#include "stdafx.h"
#include "functionsAstro.h"

std::vector<double> diffrotProfileAverage(const Mat& flow, int colS)
{
	int cols = colS ? colS : flow.cols;
	std::vector<double> averageFlow(flow.rows, 0);
	for (int r = 0; r < flow.rows; r++)
	{
		for (int c = flow.cols - cols; c < flow.cols; c++)
		{
			averageFlow[r] += flow.at<double>(r, c);
		}
		averageFlow[r] /= cols;
	}
	return averageFlow;
}

double absoluteSubpixelRegistrationError(IPCsettings& set, const Mat& src, double noisestddev, double maxShiftRatio, double accuracy)
{
	double returnVal = 0;
	Mat srcCrop1, srcCrop2;
	Mat crop1, crop2;
	//vector<double> startFractionX = { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 };
	//vector<double> startFractionY = { 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7 };
	vector<double> startFractionX = { 0.5 };
	vector<double> startFractionY = { 0.5 };
	int trials = ceil((double)set.getcols() * maxShiftRatio / accuracy) + 1;
	for (int startposition = 0; startposition < startFractionX.size(); startposition++)
	{
		int startX = src.cols * startFractionX[startposition];
		int startY = src.rows * startFractionY[startposition];
		srcCrop1 = roicrop(src, startX, startY, 1.5*set.getcols() + 2, 1.5*set.getcols() + 2);
		crop1 = roicrop(srcCrop1, srcCrop1.cols / 2, srcCrop1.rows / 2, set.getcols(), set.getcols());
		for (int iterPic = 0; iterPic < trials; iterPic++)
		{
			double shiftX = (double)iterPic / (trials - 1) * (double)set.getcols() * maxShiftRatio;
			double shiftY = 0.;
			Mat T = (Mat_<float>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
			warpAffine(srcCrop1, srcCrop2, T, cv::Size(srcCrop1.cols, srcCrop1.rows));
			crop2 = roicrop(srcCrop2, srcCrop2.cols / 2, srcCrop2.rows / 2, set.getcols(), set.getcols());
			if (noisestddev)
			{
				crop1.convertTo(crop1, CV_32F);
				crop2.convertTo(crop2, CV_32F);
				Mat noise1 = Mat::zeros(crop1.rows, crop1.cols, CV_32F);
				Mat noise2 = Mat::zeros(crop1.rows, crop1.cols, CV_32F);
				randn(noise1, 0, noisestddev);
				randn(noise2, 0, noisestddev);
				crop1 += noise1;
				crop2 += noise2;
			}
			auto shift = phasecorrel(crop1, crop2, set);
			returnVal += abs(shift.x - shiftX);
			if (0)//DEBUG
			{
				showimg(crop1, "crop1");
				showimg(crop1, "crop2");
			}
		}
	}
	returnVal /= trials;
	returnVal /= startFractionX.size();
	return returnVal;
}

double IPCparOptFun(std::vector<double>& args, const IPCsettings& settingsMaster, const Mat& source, double noisestddev, double maxShiftRatio, double accuracy)
{
	IPCsettings settings = settingsMaster;
	settings.setBandpassParameters(args[0], args[1]);
	if (args.size() > 2) settings.L2size = max(3., abs(round(args[2])));
	if (args.size() > 3) settings.applyWindow = args[3] > 0 ? true : false;
	return absoluteSubpixelRegistrationError(settings, source, noisestddev, maxShiftRatio, accuracy);
}

void optimizeIPCParameters(const IPCsettings& settingsMaster, std::string pathInput, std::string pathOutput, double maxShiftRatio, double accuracy, unsigned runs, Logger* logger)
{
	std::ofstream listing(pathOutput, std::ios::out | std::ios::app);

	listing << "Running IPC parameter optimization (" << currentDateTime() << ")" << endl;
	listing << "filename,size,stdevLmul,stdevHmul,L2,window,avgError,dateTime" << endl;
	Mat pic = loadImage(pathInput);
	std::string windowname = "objective function source";
	showimg(pic, windowname);
	double noisestdev = 0;
	auto f = [&](std::vector<double>& args) {return IPCparOptFun(args, settingsMaster, pic, noisestdev, maxShiftRatio, accuracy); };

	for (int iterOpt = 0; iterOpt < runs; iterOpt++)
	{
		Evolution Evo(4);
		Evo.NP = 24;
		Evo.mutStrat = Evolution::MutationStrategy::RAND1;
		Evo.lowerBounds = vector<double>{ 0,0,3,-1 };
		Evo.upperBounds = vector<double>{ 10,200,15,1 };
		auto Result = Evo.optimize(f, logger);
		listing << pathInput << "," << settingsMaster.getcols() << "," << Result[0] << "," << Result[1] << "," << Result[2] << "," << Result[3] << "," << f(Result) << "," << currentDateTime() << endl;
	}
	destroyWindow(windowname);
}

void optimizeIPCParametersForAllWavelengths(const IPCsettings& settingsMaster, double maxShiftRatio, double accuracy, unsigned runs, Logger* logger)
{
	std::ofstream listing("D:\\MainOutput\\IPC_parOpt.csv", std::ios::out | std::ios::trunc);
	if (1)//OPT 4par
	{
		listing << "Running IPC parameter optimization (" << currentDateTime() << "), image size " << settingsMaster.getcols() << endl;
		listing << "wavelength,stdevLmul,stdevHmul,L2,window,avgError,dateTime" << endl;
		for (int wavelength = 0; wavelength < WAVELENGTHS_STR.size(); wavelength++)
		{
			std::string path = "D:\\MainOutput\\png\\" + WAVELENGTHS_STR[wavelength] + "_proc.png";
			cout << "OPT .png load path: " << path << endl;
			Mat pic = imread(path, IMREAD_ANYDEPTH);
			showimg(pic, "objfun current source");
			if (1)//optimize
			{
				auto f = [&](std::vector<double>& args) {return IPCparOptFun(args, settingsMaster, pic, STDDEVS[wavelength], maxShiftRatio, accuracy); };
				for (int iterOpt = 0; iterOpt < runs; iterOpt++)
				{
					Evolution Evo(4);
					Evo.NP = 25;
					Evo.mutStrat = Evolution::MutationStrategy::RAND1;
					Evo.optimalFitness = 0;
					Evo.lowerBounds = vector<double>{ 0,0,3,-1 };
					Evo.upperBounds = vector<double>{ 20,200,19,1 };
					auto Result = Evo.optimize(f, logger);
					listing << WAVELENGTHS_STR[wavelength] << "," << Result[0] << "," << Result[1] << "," << Result[2] << "," << Result[3] << "," << f(Result) << "," << currentDateTime() << endl;
				}
			}
		}
	}
}

void calculateDiffrotProfile(const IPCsettings& set, const IPCsettings& set1, const IPCsettings& set2, FITStime& FITS_time, DiffrotResults* MainResults, bool twoCorrels, int itersPic, int itersX, int itersY, int itersMedian, int strajdPic, int deltaPic, int verticalFov, int deltasec, string pathMasterOut, Logger* logger, AbstractPlot1D* plt)
{
	if (logger) logger->Log("Starting IPC MainFlow calculation", SUBEVENT);
	if (plt) plt->setAxisNames("latitude [deg]", "omega [rad/s]", std::vector<std::string>{"predicted", "measured"});
	//2D stuff
	Mat omegasMainX = Mat::zeros(itersY, itersX*itersPic, CV_64F);
	Mat omegasMainY = Mat::zeros(itersY, itersX*itersPic, CV_64F);
	Mat picture = Mat::zeros(itersY, itersX*itersPic, CV_64F);
	//1D stuff
	std::vector<double> omegasAverageX1(itersY, 0);
	std::vector<double> omegasAverageX2(itersY, 0);
	std::vector<double> omegasAverageX(itersY, 0);
	std::vector<double> omegasAverageY(itersY, 0);
	std::vector<double> thetasAverage(itersY, 0);
	std::vector<double> omegasPredicted(itersY, 0);
	//1D stuff for realtime average plotting
	std::vector<double> pltXaccum(itersY, 0);
	std::vector<double> pltXavg(itersY, 0);
	std::vector<std::vector<double>> pltYaccum = zerovect2(2, itersY);
	std::vector<std::vector<double>> pltYavg = zerovect2(2, itersY);

	//omp_set_num_threads(6);
	int plusminusbufer = 6;//even!
	bool succload;
	Mat pic1, pic2;
	fitsParams params1, params2;
	int itersSucc = 0;

	for (int iterPic = 0; iterPic < itersPic; iterPic++)//main cycle - going through pairs of pics
	{
		logger->Log("Calculating picture pair " + to_string(iterPic + 1) + " / " + itersPic, SUBEVENT);
		//pic1
		FITS_time.advanceTime((bool)iterPic*timeDelta*(strajdPic - deltaPic));
		logger->Log("Loading file '" + FITS_time.path() + "'...", INFO);
		pic1 = loadfits(FITS_time.path(), params1);
		if (!params1.succload)
		{
			for (int pm = 1; pm < plusminusbufer; pm++)//try plus minus some seconds
			{
				logger->Log("Load failed, trying plusminus " + to_string(pm) + ": ", WARN);
				if (pm % 2 == 0)
					FITS_time.advanceTime(pm);
				else
					FITS_time.advanceTime(-pm);
				pic1 = loadfits(FITS_time.path(), params1);
				if (params1.succload)
					break;
			}
		}
		if (!params1.succload)
		{
			logger->Log("Picture not loaded succesfully - stopping current block execution ", FATAL);
			FITS_time.advanceTime(plusminusbufer / 2);//fix unsuccesful plusminus
		}

		//pic2
		FITS_time.advanceTime(deltasec);
		logger->Log("Loading file '" + FITS_time.path() + "'...", INFO);
		pic2 = loadfits(FITS_time.path(), params2);
		if (!params2.succload)
		{
			for (int pm = 1; pm < plusminusbufer; pm++)//try plus minus some seconds
			{
				logger->Log("Load failed, trying plusminus " + to_string(pm) + ": ", WARN);
				if (pm % 2 == 0)
					FITS_time.advanceTime(pm);
				else
					FITS_time.advanceTime(-pm);
				pic2 = loadfits(FITS_time.path(), params2);
				if (params2.succload)
					break;
			}
		}
		if (!params2.succload)
		{
			logger->Log("Picture not loaded succesfully - stopping current block execution ", FATAL);
			FITS_time.advanceTime(plusminusbufer / 2);//fix unsuccesful plusminus
		}
		succload = params1.succload && params2.succload;
		if (succload)//load succesfull
		{
			itersSucc++;

			if (iterPic == 0)
			{
				showimg(pic1, "diffrot pic 1");
				showimg(pic2, "diffrot pic 2");
			}

			//average fits values for pics 1 and 2
			double R = (params1.R + params2.R) / 2;
			double theta0 = (params1.theta0 + params2.theta0) / 2;
			int vertikalniskok = verticalFov / itersY;//px vertical jump
			int vertikalniShift = 0;// 600;//abych videl sunspot hezky - 600

			for (int iterX = 0; iterX < itersX; iterX++)//X cyklus
			{
				#pragma omp parallel for
				for (int iterY = 0; iterY < itersY; iterY++)//Y cyklus
				{
					Mat crop1, crop2;
					Point2d shift(0, 0), shift1, shift2;
					double predicted_omega = 0, predicted_phi = 0, theta = 0, phi_x = 0, phi_x1 = 0, phi_x2 = 0, omega_x = 0, omega_x1 = 0, omega_x2 = 0, phi_y = 0, omega_y = 0, beta = 0;
					theta = asin(((double)vertikalniskok*(itersY / 2 - iterY) - vertikalniShift) / R) + theta0;//latitude
					predicted_omega = (14.713 - 2.396*pow(sin(theta), 2) - 1.787*pow(sin(theta), 4)) / (24. * 60. * 60.) / (360. / 2. / PI);//predicted omega in radians per second
					predicted_phi = predicted_omega * (double)deltasec;//predicted shift in radians

					//reduce shift noise by computing multiple shifts near central meridian and then working with their median
					std::vector<Point2d> shifts(itersMedian);
					std::vector<Point2d> shifts1(itersMedian);
					std::vector<Point2d> shifts2(itersMedian);
					for (int iterMedian = 0; iterMedian < itersMedian; iterMedian++)
					{
						//crop1
						crop1 = roicrop(pic1, params1.fitsMidX - itersX / 2 + iterX - itersMedian / 2 + iterMedian, params1.fitsMidY + vertikalniskok * (- itersY / 2 + iterY) + vertikalniShift, set.getcols(), set.getrows());
						//crop2
						crop2 = roicrop(pic2, params2.fitsMidX - itersX / 2 + iterX - itersMedian / 2 + iterMedian, params2.fitsMidY + vertikalniskok * (- itersY / 2 + iterY) + vertikalniShift, set.getcols(), set.getrows());

						if (twoCorrels)
						{
							shifts1[iterMedian] = phasecorrel(crop1, crop2, set1);
							shifts2[iterMedian] = phasecorrel(crop1, crop2, set2);
						}
						else
						{
							shifts[iterMedian] = phasecorrel(crop1, crop2, set);
						}
					}

					//calculate omega from shift
					if (twoCorrels)
					{
						shift1 = median(shifts1);
						shift2 = median(shifts2);
						phi_x1 = asin(shift1.x / (R*cos(theta)));
						phi_x2 = asin(shift2.x / (R*cos(theta)));
						omega_x1 = phi_x1 / deltasec;
						omega_x2 = phi_x2 / deltasec;
						omegasAverageX1[iterY] += omega_x1;
						omegasAverageX2[iterY] += omega_x2;
					}
					else
					{
						shift = median(shifts);
						phi_x = asin(shift.x / (R*cos(theta)));
						omega_x = phi_x / deltasec;
						omegasAverageX[iterY] += omega_x;
						pltXaccum[iterY] += theta;//plt
						pltYaccum[0][iterY] += predicted_omega;//plt
						pltYaccum[1][iterY] += omega_x;//plt
					}

					phi_y = (theta - theta0) - atan((R*sin(theta - theta0) - shift.y) / (R*cos(theta - theta0)));
					omega_y = phi_y / deltasec;
					omegasAverageY[iterY] += omega_y;
					thetasAverage[iterY] += theta;
					omegasPredicted[iterY] += predicted_omega;

					pltXavg[iterY] = pltXaccum[iterY] / itersSucc;//plt
					pltYavg[0][iterY] = pltYaccum[0][iterY] / itersSucc;//plt
					pltYavg[1][iterY] = pltYaccum[1][iterY] / itersSucc;//plt

					omegasMainX.at<double>(iterY, (itersPic - 1)*itersX - iterPic * itersX - iterX) = omega_x;
					omegasMainY.at<double>(iterY, (itersPic - 1)*itersX - iterPic * itersX - iterX) = omega_y;
					picture.at<double>(iterY, (itersPic - 1)*itersX - iterPic * itersX - iterX) = pic1.at<ushort>(params1.fitsMidY + vertikalniskok * (iterY - floor((double)itersY / 2.)) + vertikalniShift, params1.fitsMidX - floor((double)itersX / 2.) + iterX);
				}//Y for cycle end
			}//X for cycle end
		}//load succesfull
		plt->plot(pltXavg, pltYavg);
	}//picture pairs cycle end

	cout << "> processing results ... " << endl;

	for (int iterPic = 0; iterPic < itersY; iterPic++)//compute averages
	{
		if (twoCorrels)
		{
			omegasAverageX1[iterPic] /= (itersX*itersSucc);
			omegasAverageX2[iterPic] /= (itersX*itersSucc);
		}
		else
		{
			omegasAverageX[iterPic] /= (itersX*itersSucc);
		}

		omegasAverageY[iterPic] /= (itersX*itersSucc);
		thetasAverage[iterPic] /= (itersX*itersSucc);
		omegasPredicted[iterPic] /= (itersX*itersSucc);
		thetasAverage[iterPic] *= (360. / 2. / PI);//deg
	}

	MainResults->FlowX = omegasMainX;
	MainResults->FlowY = omegasMainY;
	MainResults->FlowPic = picture;
	if (logger) logger->Log("IPC MainFlow calculation finished", SUBEVENT);
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateLinearSwindFlow(const IPCsettings& set, std::string path, double SwindCropFocusX, double SwindCropFocusY)
{
	//load pics
	std::vector<Mat> pics(SwindPicCnt);
	for (int iterPic = 0; iterPic < SwindPicCnt; iterPic++)
	{
		pics[iterPic] = imread(path + "0" + to_string(iterPic + 1) + "_calib.PNG", IMREAD_ANYDEPTH);
		pics[iterPic] = roicrop(pics[iterPic], SwindCropFocusX*pics[iterPic].cols, SwindCropFocusY*pics[iterPic].rows, set.getcols(), set.getrows());
		//saveimg(path + "cropped//crop" + to_string(iterPic) + ".PNG", pics[iterPic], false, cv::Size2i(5*pics[iterPic].cols, 5*pics[iterPic].rows));
	}
	
	//calculate shifts
	auto shiftsX = zerovect(SwindPicCnt);
	auto shiftsY = zerovect(SwindPicCnt);
	auto indices = iota(0, SwindPicCnt);

	for (int iterPic = 0; iterPic < SwindPicCnt; iterPic++)
	{
		auto shift = phasecorrel(pics[0], pics[iterPic], set);
		shiftsX[iterPic] = shift.x;
		shiftsY[iterPic] = shift.y;
		indices[iterPic] = iterPic;
	}
	for (int iterPic = 0; iterPic < 3; iterPic++)//starting things merge with zero peak - omit
	{
		//shiftsX[iterPic] = 0;
		//shiftsY[iterPic] = 0;
	}
	return std::make_tuple(shiftsX, shiftsY, indices);
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateConstantSwindFlow(const IPCsettings& set, std::string path, double SwindCropFocusX, double SwindCropFocusY)
{
	//load pics
	std::vector<Mat> pics(SwindPicCnt);
	for (int iterPic = 0; iterPic < SwindPicCnt; iterPic++)
	{
		pics[iterPic] = imread(path + "0" + to_string(iterPic + 1) + "_calib.PNG", IMREAD_ANYDEPTH);
		pics[iterPic] = roicrop(pics[iterPic], SwindCropFocusX*pics[iterPic].cols, SwindCropFocusY*pics[iterPic].rows, set.getcols(), set.getrows());
		//saveimg(path + "cropped//crop" + to_string(iterPic) + ".PNG", pics[iterPic], false, cv::Size2i(2000, 2000));
	}

	//calculate shifts
	auto shiftsX = zerovect(SwindPicCnt - 1);
	auto shiftsY = zerovect(SwindPicCnt - 1);
	auto indices = iota(1, SwindPicCnt - 1);

	for (int iterPic = 0; iterPic < SwindPicCnt - 1; iterPic++)
	{
		auto shift = phasecorrel(pics[iterPic], pics[iterPic + 1], set);
		shiftsX[iterPic] = shift.x;
		shiftsY[iterPic] = shift.y;
		indices[iterPic] = iterPic;
	}

	return std::make_tuple(shiftsX, shiftsY, indices);
}
