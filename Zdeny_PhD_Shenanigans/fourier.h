#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"

using namespace std;
using namespace cv;

Mat fourier(const Mat& sourceimgIn);

Mat fourierinv(const Mat& realIn, const Mat& imagIn);

Mat quadrantswap(const Mat& sourceimgDFT);

void showfourier(const Mat& DFTimgIn, bool logar = true, bool expon = false, std::string magnwindowname = "FFTmagn", std::string phasewindowname = "FFTphase");

inline Mat gaussian(int rows, int cols, double stdevYmult, double stdevXmult)
{
	Mat gaussian = Mat::ones(rows, cols, CV_64F);
	double centerX = floor((double)cols / 2);
	double centerY = floor((double)rows / 2);
	int x, y;
	for (y = 0; y < rows; y++)
	{
		for (x = 0; x < cols; x++)
		{
			gaussian.at<double>(y, x) = std::exp(-(std::pow(x - centerX, 2) / 2 / std::pow((double)cols / stdevXmult, 2) + std::pow(y - centerY, 2) / 2 / std::pow((double)rows / stdevYmult, 2)));
		}
	}
	normalize(gaussian, gaussian, 0, 1, CV_MINMAX);
	return gaussian;
}

inline Mat laplacian(int rows, int cols, double stdevYmult, double stdevXmult)
{
	Mat laplacian = Mat::ones(rows, cols, CV_64F);
	laplacian = 1 - gaussian(rows, cols, stdevYmult, stdevXmult);
	normalize(laplacian, laplacian, 0, 1, CV_MINMAX);
	return laplacian;
}

inline Mat bandpassian(int rows, int cols, double stdevLmult, double stdevHmult)
{
	Mat bandpassian = gaussian(rows, cols, stdevLmult, stdevLmult).mul(laplacian(rows, cols, stdevHmult, stdevHmult));
	normalize(bandpassian, bandpassian, 0, 1, CV_MINMAX);
	return bandpassian;
}

Mat edgemask(int rows, int cols);

Mat sinian(int rows, int cols, double frequencyX, double frequencyY);

Mat bandpass(const Mat& sourceimgDFTIn, const Mat& bandpassMat);

Mat convolute(Mat sourceimg, Mat PSFimg);

Mat deconvolute(Mat sourceimg, Mat PSFimg);

Mat deconvoluteWiener(const Mat& sourceimg, const Mat& PSFimg);

Mat frequencyFilter(const Mat& sourceimg, const Mat& mask);
