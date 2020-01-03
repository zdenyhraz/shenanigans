//auxiliary improc functions and features
//updated frequently @ https://github.com/zdenyhraz
//PhD work of Zdenek Hrazdira
//made during 2018-2019

#include "stdafx.h"
#include "functionsBaseCV.h"

std::tuple<double, double> minMaxMat(const Mat& sourceimg)
{
	double minR, maxR;
	minMaxLoc(sourceimg, &minR, &maxR, nullptr, nullptr);
	return make_tuple(minR, maxR);
}

std::vector<double> polyfit(const std::vector<double>& data, int degree)
{
	int dataCount = data.size();
	Mat X = Mat::zeros(dataCount, degree + 1, CV_64F);//matice planu
	Mat Y = Mat::zeros(dataCount, 1, CV_64F);//matice prave strany
	for (int r = 0; r < X.rows; r++)
	{
		Y.at<double>(r, 0) = data[r];
		for (int c = 0; c < X.cols; c++)
		{
			if (!c)
				X.at<double>(r, c) = 1;
			else
				X.at<double>(r, c) = pow(r, c);
		}
	}
	Mat coeffs = (X.t()*X).inv()*X.t()*Y;//least squares
	Mat fitM = X * coeffs;
	std::vector<double> fit(dataCount, 0);
	for (int r = 0; r < fitM.rows; r++)
	{
		fit[r] = fitM.at<double>(r, 0);
	}
	return fit;
}

Mat crosshair(const Mat& sourceimgIn, cv::Point point)
{
	Mat sourceimg = sourceimgIn.clone();
	Scalar CrosshairColor = Scalar(0.3);
	int thickness = max(sourceimg.cols / 250, 1);
	int inner = sourceimg.cols / 40;
	int outer = sourceimg.cols / 15;
	cv::Point offset1(0, outer);
	cv::Point offset2(outer, 0);
	cv::Point offset3(0, inner);
	cv::Point offset4(inner, 0);
	sourceimg.convertTo(sourceimg, CV_64F);
	normalize(sourceimg, sourceimg, 0, 1, CV_MINMAX);

	circle(sourceimg, point, inner, CrosshairColor, thickness, 0, 0);
	line(sourceimg, point + offset3, point + offset1, CrosshairColor, thickness);
	line(sourceimg, point + offset4, point + offset2, CrosshairColor, thickness);
	line(sourceimg, point - offset3, point - offset1, CrosshairColor, thickness);
	line(sourceimg, point - offset4, point - offset2, CrosshairColor, thickness);
	return sourceimg;
}

Mat xko(const Mat& sourceimgIn, cv::Point point, Scalar CrosshairColor, std::string inputType)
{
	Mat sourceimg = sourceimgIn.clone();
	int inner = 0;
	int outer = 4;
	cv::Point offset1(-outer, -outer);
	cv::Point offset2(outer, -outer);
	cv::Point offset3(outer, outer);
	cv::Point offset4(-outer, outer);
	Mat sourceimgCLR = Mat::zeros(sourceimg.size(), CV_16UC3);
	if (inputType == "CLR")
	{
		normalize(sourceimg, sourceimg, 0, 65535, CV_MINMAX);
		sourceimg.convertTo(sourceimg, CV_16UC3);
		sourceimg.copyTo(sourceimgCLR);
	}
	if (inputType == "GRS")
	{
		normalize(sourceimg, sourceimg, 0, 65535, CV_MINMAX);
		sourceimg.convertTo(sourceimg, CV_16UC1);
		for (int r = 0; r < sourceimg.rows; r++)
		{
			for (int c = 0; c < sourceimg.cols; c++)
			{
				sourceimgCLR.at<Vec3w>(r, c)[0] = sourceimg.at<ushort>(r, c);
				sourceimgCLR.at<Vec3w>(r, c)[1] = sourceimg.at<ushort>(r, c);
				sourceimgCLR.at<Vec3w>(r, c)[2] = sourceimg.at<ushort>(r, c);
			}
		}
	}
	double THICCness = 1;
	//circle(sourceimgCLR, point, inner, CrosshairColor, 2);
	circle(sourceimgCLR, point, 5, CrosshairColor, THICCness, 0, 0);
	line(sourceimgCLR, point + offset3, point + offset1, CrosshairColor, THICCness);
	line(sourceimgCLR, point + offset4, point + offset2, CrosshairColor, THICCness);
	line(sourceimgCLR, point - offset3, point - offset1, CrosshairColor, THICCness);
	line(sourceimgCLR, point - offset4, point - offset2, CrosshairColor, THICCness);
	return sourceimgCLR;
}

Mat pointik(const Mat& sourceimgIn, cv::Point point, Scalar CrosshairColor, std::string inputType)
{
	Mat sourceimg = sourceimgIn.clone();
	int inner = 1;
	int outer = 0;
	cv::Point offset1(-outer, -outer);
	cv::Point offset2(outer, -outer);
	cv::Point offset3(outer, outer);
	cv::Point offset4(-outer, outer);
	Mat sourceimgCLR = Mat::zeros(sourceimg.size(), CV_16UC3);
	if (inputType == "CLR")
	{
		normalize(sourceimg, sourceimg, 0, 65535, CV_MINMAX);
		sourceimg.convertTo(sourceimg, CV_16UC1);
		sourceimg.copyTo(sourceimgCLR);
	}
	if (inputType == "GRS")
	{
		normalize(sourceimg, sourceimg, 0, 65535, CV_MINMAX);
		sourceimg.convertTo(sourceimg, CV_16UC1);
		for (int r = 0; r < sourceimg.rows; r++)
		{
			for (int c = 0; c < sourceimg.cols; c++)
			{
				sourceimgCLR.at<Vec3w>(r, c)[0] = sourceimg.at<ushort>(r, c);
				sourceimgCLR.at<Vec3w>(r, c)[1] = sourceimg.at<ushort>(r, c);
				sourceimgCLR.at<Vec3w>(r, c)[2] = sourceimg.at<ushort>(r, c);
			}
		}
	}
	circle(sourceimgCLR, point, inner, CrosshairColor, 2);
	return sourceimgCLR;
}

Mat roicrop(const Mat& sourceimgIn, int x, int y, int w, int h)//x,y souradnice stredu, cols, rows
{
	Rect roi = Rect(x - std::floor((double)w / 2.), y - std::floor((double)h / 2.), w, h);
	Mat crop = sourceimgIn(roi);
	return crop.clone();
}

Mat kirkl(unsigned size)
{
	Mat kirkl = Mat::zeros(size, size, CV_64F);
	for (int r = 0; r < size; r++)
	{
		for (int c = 0; c < size; c++)
		{
			if ((sqr((double)r - floor(size / 2)) + sqr((double)c - floor(size / 2))) < sqr((double)size / 2 + 1))
			{
				kirkl.at<double>(r, c) = 1.;
			}
			else
			{
				kirkl.at<double>(r, c) = 0.;
			}
		}
	}
	return kirkl;
}

Mat kirkl(int rows, int cols, unsigned radius)
{
	Mat kirkl = Mat::zeros(rows, cols, CV_64F);
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			if ((sqr((double)r - floor(rows / 2)) + sqr((double)c - floor(cols / 2))) < sqr(radius))
			{
				kirkl.at<double>(r, c) = 1.;
			}
			else
			{
				kirkl.at<double>(r, c) = 0.;
			}
		}
	}
	return kirkl;
}

Mat kirklcrop(const Mat& sourceimgIn, int x, int y, int diameter)
{
	Mat crop = roicrop(sourceimgIn, x, y, diameter, diameter);
	Mat kirklik = kirkl(diameter);
	return crop.mul(kirklik);
}

Point2d findCentroidDouble(const Mat& sourceimg)
{
	double M00 = 0.0;
	double M01 = 0.0;
	double M10 = 0.0;
	for (int r = 0; r < sourceimg.rows; r++)
	{
		//std::cout << " FCrow " << r;
		for (int c = 0; c < sourceimg.cols; c++)
		{
			M00 += 1.0 * sourceimg.at<double>(r, c);
			M01 += (double)r * sourceimg.at<double>(r, c);
			M10 += (double)c * sourceimg.at<double>(r, c);
		}
	}
	Point2d CentroidDouble(2, 0.0);
	CentroidDouble.x = M10 / M00;//double output
	CentroidDouble.y = M01 / M00;
	return CentroidDouble;
}

Mat combineTwoPics(const Mat& source1In, const Mat& source2In, CombinePicsStyle style, double sigma)
{
	Mat source1 = source1In.clone();
	Mat source2 = source2In.clone();
	Mat result = Mat::zeros(source1.rows, source1.cols, CV_8UC3);
	source1.convertTo(source1, CV_64F);
	source2.convertTo(source2, CV_64F);
	if (style == HUEBRIGHT)
	{
		normalize(source1, source1, 0, 1, CV_MINMAX);
		normalize(source2, source2, 0, 1, CV_MINMAX);
		pow(source2, 2, source2);
		for (int r = 0; r < source1.rows; r++)
		{
			for (int c = 0; c < source1.cols; c++)
			{
				auto BGRjet = colorMapJET(255.*source1.at<double>(r, c));
				auto HUE = BGR_to_HUE(BGRjet);
				auto BGR = HUE_to_BGR(HUE, 255.*source2.at<double>(r, c), BGRjet);

				result.at<Vec3b>(r, c)[0] = std::get<0>(BGR);
				result.at<Vec3b>(r, c)[1] = std::get<1>(BGR);
				result.at<Vec3b>(r, c)[2] = std::get<2>(BGR);
			}
		}
	}
	if (style == BINARYBLUERED)
	{
		normalize(source2, source2, 0, 1, CV_MINMAX);
		auto minMax = minMaxMat(source1);
		for (int r = 0; r < source1.rows; r++)
		{
			for (int c = 0; c < source1.cols; c++)
			{
				auto BGR = colorMapBINARY(source1.at<double>(r, c), std::get<0>(minMax), std::get<1>(minMax), sigma);
				result.at<Vec3b>(r, c)[0] = source2.at<double>(r, c)*std::get<0>(BGR);
				result.at<Vec3b>(r, c)[1] = source2.at<double>(r, c)*std::get<1>(BGR);
				result.at<Vec3b>(r, c)[2] = source2.at<double>(r, c)*std::get<2>(BGR);
			}
		}
	}
	return result;
}

Mat applyColorMapZdeny(const Mat& sourceimgIn, double quantileB, double quantileT, bool color)
{
	Mat sourceimg = sourceimgIn.clone();
	//input is grayscale, output is colored
	int caxisMin = 0, caxisMax = 255;

	normalize(sourceimg, sourceimg, 0, 255, CV_MINMAX);
	sourceimg.convertTo(sourceimg, CV_8U);

	if ((quantileB > 0) || (quantileT < 1))
	{
		vector<int> picvalues(sourceimg.rows*sourceimg.cols, 0);
		for (int r = 0; r < sourceimg.rows; r++)
		{
			for (int c = 0; c < sourceimg.cols; c++)
			{
				picvalues[r*sourceimg.cols + c] = sourceimg.at<uchar>(r, c);
			}
		}

		//sort the vector
		sort(picvalues.begin(), picvalues.end());

		//calculate the alpha and 1-alpha quantiles
		int alpha1Quantile = picvalues[round(quantileB*(picvalues.size() - 1))];
		int alpha2Quantile = picvalues[round(quantileT*(picvalues.size() - 1))];
		caxisMin = alpha1Quantile;
		caxisMax = alpha2Quantile;
	}

	if (color)
	{
		Mat sourceimgOutCLR(sourceimg.rows, sourceimg.cols, CV_8UC3);
		for (int r = 0; r < sourceimgOutCLR.rows; r++)
		{
			for (int c = 0; c < sourceimgOutCLR.cols; c++)
			{
				double x = sourceimg.at<uchar>(r, c);

				std::tuple<int, int, int> BGR = colorMapJET(x, caxisMin, caxisMax);
				sourceimgOutCLR.at<Vec3b>(r, c)[0] = round(std::get<0>(BGR));
				sourceimgOutCLR.at<Vec3b>(r, c)[1] = round(std::get<1>(BGR));
				sourceimgOutCLR.at<Vec3b>(r, c)[2] = round(std::get<2>(BGR));
			}
		}
		return sourceimgOutCLR;

	}
	else
	{
		Mat sourceimgOutGS(sourceimg.rows, sourceimg.cols, CV_8UC1);
		for (int r = 0; r < sourceimgOutGS.rows; r++)
		{
			for (int c = 0; c < sourceimgOutGS.cols; c++)
			{
				double x = sourceimg.at<uchar>(r, c);
				sourceimgOutGS.at<char>(r, c) = clamp(x, caxisMin, caxisMax);
			}
		}
		return sourceimgOutGS;
	}
}

void showimg(const Mat& sourceimgIn, std::string windowname, bool color, double quantileB, double quantileT, Size2i showSize)
{
	Mat sourceimg = sourceimgIn.clone();
	double RowColRatio = (double)sourceimg.rows / (double)sourceimg.cols;
	int namedWindowRows = 600;
	int namedWindowCols = (double)namedWindowRows / RowColRatio;
	namedWindow(windowname, WINDOW_NORMAL);
	if (showSize == Size2i(0, 0))
	{
		resizeWindow(windowname, namedWindowCols, namedWindowRows);
	}
	else
	{
		resizeWindow(windowname, showSize);
	}
	normalize(sourceimg, sourceimg, 0, 255, CV_MINMAX);
	sourceimg.convertTo(sourceimg, CV_8U);
	if (sourceimg.channels() == 1) sourceimg = applyColorMapZdeny(sourceimg, quantileB, quantileT, color);
	imshow(windowname, sourceimg);
	waitKey(1);
}

void saveimg(std::string path, const Mat& sourceimgIn, bool bilinear, Size2i exportSize)
{
	Mat sourceimg = sourceimgIn.clone();
	normalize(sourceimg, sourceimg, 0, 255, CV_MINMAX);
	sourceimg.convertTo(sourceimg, CV_8U);

	double RowColRatio = (double)sourceimg.rows / (double)sourceimg.cols;
	int namedWindowRows = 1200;
	int namedWindowCols = (double)namedWindowRows / RowColRatio;

	if (exportSize == Size2i(0, 0))
	{
		if (bilinear)
			resize(sourceimg, sourceimg, Size2i(namedWindowCols, namedWindowRows), 0, 0, INTER_LINEAR);
		else
			resize(sourceimg, sourceimg, Size2i(namedWindowCols, namedWindowRows), 0, 0, INTER_NEAREST);
	}
	else
	{
		if (bilinear)
			resize(sourceimg, sourceimg, exportSize, 0, 0, INTER_LINEAR);
		else
			resize(sourceimg, sourceimg, exportSize, 0, 0, INTER_NEAREST);
	}
	imwrite(path, sourceimg);
}

void saveMatToCsv(const std::string& path, const Mat& matIn)
{
	std::ofstream listing(path, std::ios::out | std::ios::trunc);
	Mat mat = matIn.clone();
	mat.convertTo(mat, CV_64F);
	for (int r = 0; r < mat.rows; r++)
	{
		for (int c = 0; c < mat.cols; c++)
		{
			listing << mat.at<double>(r, c) << ",";
		}
		listing << endl;
	}
}

void colorMapDebug(double sigmaa)
{
	int rows = 500;
	int cols = 1500;
	Mat colormap = Mat::zeros(rows, cols, CV_8UC3);
	ColorMapStyle colormapStyle = CM_BINARY;
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			std::tuple<int, int, int>BGR;

			if (colormapStyle == CM_JET)
			{
				double x = 255.*c / (cols - 1.);
				BGR = colorMapJET(x);
			}
			if (colormapStyle == CM_BINARY)
			{
				double x = 2.*((double)c / (cols - 1.)) - 1.;
				BGR = colorMapBINARY(x, -1, 1, sigmaa);
			}

			colormap.at<Vec3b>(r, c)[0] = std::get<0>(BGR);
			colormap.at<Vec3b>(r, c)[1] = std::get<1>(BGR);
			colormap.at<Vec3b>(r, c)[2] = std::get<2>(BGR);
		}
	}
	showimg(colormap, "colormap");
}


