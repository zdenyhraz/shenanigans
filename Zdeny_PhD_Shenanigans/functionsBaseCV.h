#pragma once
#include "functionsBaseSTL.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

 enum ColorMapStyle { CM_JET, CM_BINARY };
 enum CombinePicsStyle { HUEBRIGHT, BINARYBLUERED };

std::tuple<double, double> minMaxMat(const Mat& sourceimg);

std::vector<double> polyfit(std::vector<double> data, int degree);

Mat crosshair(const Mat& sourceimgIn, cv::Point point);

Mat xko(const Mat& sourceimgIn, cv::Point point, Scalar CrosshairColor, std::string inputType);

Mat pointik(const Mat& sourceimgIn, cv::Point point, Scalar CrosshairColor, std::string inputType);

Mat roicrop(const Mat& sourceimgIn, int x, int y, int w, int h);

Mat kirkl(unsigned size);

Mat kirkl(int rows, int cols, unsigned radius);

Mat kirklcrop(const Mat& sourceimgIn, int x, int y, int diameter);

Point2d findCentroidDouble(const Mat& sourceimg);

inline std::tuple<int, int, int> colorMapJET(int x, int caxisMin=0, int caxisMax=255)
{
	double B, G, R;
	double sh = 0.125 * (caxisMax - caxisMin);
	double start = caxisMin;
	double mid = caxisMin + 0.5*(caxisMax - caxisMin);
	double end = caxisMax;

	B = (x > (start + sh)) ? clamp(-255 / 2 / sh * x + 255 / 2 / sh * (mid + sh), 0, 255) : (x < start ? 255 / 2 : clamp(255 / 2 / sh * x + 255 / 2 - 255 / 2 / sh * start, 0, 255));
	G = (x < mid) ? clamp(255 / 2 / sh * x - 255 / 2 / sh * (start + sh), 0, 255) : clamp(-255 / 2 / sh * x + 255 / 2 / sh * (end - sh), 0, 255);
	R = (x < (end - sh)) ? clamp(255 / 2 / sh * x - 255 / 2 / sh * (mid - sh), 0, 255) : (x > end ? 255 / 2 : clamp(-255 / 2 / sh * x + 255 / 2 + 255 / 2 / sh * end, 0, 255));

	return std::make_tuple(B, G, R);
}

inline std::tuple<int, int, int> colorMapBINARY(double x, double caxisMin=-1, double caxisMax=1, double sigma=1)
{
	double B, G, R;

	if (sigma == 0)//linear
	{
		if (x < 0)
		{
			B = 255;
			G = 255. / -caxisMin * x + 255;
			R = 255. / -caxisMin * x + 255;
		}
		else
		{
			B = 255 - 255. / caxisMax * x;
			G = 255 - 255. / caxisMax * x;
			R = 255;
		}
	}
	else//gaussian
	{
		double delta = caxisMax - caxisMin;
		if (x < 0)
		{
			B = 255;
			G = gaussian1D(x, 255, 0, sigma*delta / 10);
			R = gaussian1D(x, 255, 0, sigma*delta / 10);
		}
		else
		{
			B = gaussian1D(x, 255, 0, sigma*delta / 10);
			G = gaussian1D(x, 255, 0, sigma*delta / 10);
			R = 255;
		}
	}

	return std::make_tuple(B, G, R);
}

inline std::tuple<double, double, double> BGR_to_HUE(std::tuple<int, int, int> BGR)
{
	double ratioBG = (double)std::get<0>(BGR) / std::get<1>(BGR);
	double ratioGR = (double)std::get<1>(BGR) / std::get<2>(BGR);
	double ratioRB = (double)std::get<2>(BGR) / std::get<0>(BGR);
	return std::make_tuple(ratioBG, ratioGR, ratioRB);
}

inline std::tuple<int, int, int> HUE_to_BGR(std::tuple<double, double, double> HUE, double brightness, std::tuple<int, int, int> BGRsource)
{
	int B = 1, G = 1, R = 1;

	if (std::get<0>(HUE) == 0) B = 0;//B is 0
	if (std::get<1>(HUE) == 0) G = 0;//G is 0
	if (std::get<2>(HUE) == 0) R = 0;//R is 0

	if (isnan(std::get<0>(HUE)))//R idk
	{
		B = 0;
		G = 0;
		R = brightness / 255 * std::get<2>(BGRsource);
	}
	else if (isnan(std::get<1>(HUE)))//B idk
	{
		G = 0;
		R = 0;
		B = brightness / 255 * std::get<0>(BGRsource);
	}
	else if (isnan(std::get<2>(HUE)))//G idk
	{
		R = 0;
		B = 0;
		G = brightness / 255 * std::get<1>(BGRsource);
	}
	else if (std::get<0>(HUE) > 1)//max is B or R
	{
		if (std::get<1>(HUE) > 1)//B is max
		{
			if (B) B = brightness;
			if (R) R = std::get<2>(HUE)*B;
			if (G) G = B / std::get<0>(HUE);
		}
		else//R is max
		{
			if (R) R = brightness;
			if (G) G = std::get<1>(HUE)*R;
			if (B) B = R / std::get<2>(HUE);
		}
	}
	else if (std::get<0>(HUE) <= 1)//max is G or R
	{
		if (std::get<1>(HUE) > 1)//G is max
		{
			if (G) G = brightness;
			if (R) R = G / std::get<1>(HUE);
			if (B) B = std::get<0>(HUE)*G;
		}
		else//R is max
		{
			if (R) R = brightness;
			if (G) G = std::get<1>(HUE)*R;
			if (B) B = R / std::get<2>(HUE);
		}
	}

	return std::make_tuple(B, G, R);
}

Mat combineTwoPics(const Mat& source1In, const Mat& source2In, CombinePicsStyle style, double sigma = 1);

Mat applyColorMapZdeny(const Mat& sourceimgIn, double quantileB = 0, double quantileT = 1, bool color = true);

Mat matFromVector(std::vector<double> vec, int cols);

void showimg(const Mat& sourceimgIn, std::string windowname, bool color = false, double quantileB = 0, double quantileT = 1, Size2i showSize = Size2i(0, 0));

void saveimg(std::string path, const Mat& sourceimgIn, bool bilinear = false, Size2i exportSize = Size2i(0, 0));

void saveMatToCsv(const std::string& path, const Mat& matIn);

inline Point2d median(std::vector<Point2d>& vec)
{
	//function changes the vec order, watch out
	std::sort(vec.begin(), vec.end(), [](Point2d a, Point2d b) {return a.x < b.x; });
	return vec[vec.size() / 2];
}

inline Mat vectToMat(std::vector<double>& vec)
{
	return Mat(vec).reshape(0, vec.size());
}

inline std::vector<Mat> vect2ToMats(std::vector<std::vector<double>>& vec)
{
	std::vector<Mat> result(vec.size());
	for (int i = 0; i < vec.size(); i++)
		result[i] = Mat(vec[i]).reshape(0, vec[i].size());
	return result;
}

inline std::vector<double> mat1ToVect(const Mat& mat)
{
	std::vector<double> result(mat.rows, 0);
	for (int r = 0; r < mat.rows; r++)
		result[r] = mat.at<double>(r, 0);
	return result;
}

inline std::vector<std::vector<double>> matToVect2(const Mat& mat)
{
	std::vector<std::vector<double>> result = zerovect2(mat.rows, mat.cols);
	for (int r = 0; r < mat.rows; r++)
		for (int c = 0; c < mat.cols; c++)
			result[r][c] = mat.at<double>(r, c);
	return result;
}

void colorMapDebug(double sigmaa);

inline void exportToMATLAB(const Mat& Zdata, double xmin, double xmax, double ymin, double ymax, std::string path)
{
	std::ofstream listing(path, std::ios::out | std::ios::trunc);
	listing << xmin << "," << xmax << "," << ymin << "," << ymax << endl;
	for (int r = 0; r < Zdata.rows; r++)
		for (int c = 0; c < Zdata.cols; c++)
			listing << Zdata.at<double>(r, c) << endl;
}