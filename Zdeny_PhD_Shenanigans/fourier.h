#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"

using namespace std;
using namespace cv;

inline Mat fourier(const Mat& sourceimgIn)
{
	Mat sourceimg = sourceimgIn.clone();
	sourceimg.convertTo(sourceimg, CV_64F, 1. / 65535);
	Mat sourceimgcomplex[2] = { sourceimg, Mat::zeros(sourceimg.size(),CV_64F) };
	Mat sourceimgcomplexmerged;
	merge(sourceimgcomplex, 2, sourceimgcomplexmerged);
	dft(sourceimgcomplexmerged, sourceimgcomplexmerged);
	return sourceimgcomplexmerged;
}

inline Mat fourierinv(const Mat& realIn, const Mat& imagIn)
{
	Mat real = realIn.clone();
	Mat imag = imagIn.clone();
	Mat invDFT;
	Mat DFTcomplex[2] = { real, imag };
	Mat DFTcomplexmerged;
	merge(DFTcomplex, 2, DFTcomplexmerged);
	dft(DFTcomplexmerged, invDFT, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
	normalize(invDFT, invDFT, 0, 65535, CV_MINMAX);
	invDFT.convertTo(invDFT, CV_16UC1);
	return invDFT;
}

inline Mat quadrantswap(const Mat& sourceimgDFT)
{
	Mat centeredDFT = sourceimgDFT.clone();
	int centerX = centeredDFT.cols / 2;
	int centerY = centeredDFT.rows / 2;
	Mat q1(centeredDFT, Rect(0, 0, centerX, centerY));
	Mat q2(centeredDFT, Rect(centerX, 0, centerX, centerY));
	Mat q3(centeredDFT, Rect(0, centerY, centerX, centerY));
	Mat q4(centeredDFT, Rect(centerX, centerY, centerX, centerY));
	Mat temp;

	q1.copyTo(temp);
	q4.copyTo(q1);
	temp.copyTo(q4);

	q2.copyTo(temp);
	q3.copyTo(q2);
	temp.copyTo(q3);
	return centeredDFT;
}

void showfourier(const Mat& DFTimgIn, bool logar = true, bool expon = false, std::string magnwindowname = "FFTmagn", std::string phasewindowname = "FFTphase");

inline Mat edgemask(int rows, int cols)
{
	Mat edgemask;
	createHanningWindow(edgemask, cv::Size(cols, rows), CV_64F);
	return edgemask;
}

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

Mat sinian(int rows, int cols, double frequencyX, double frequencyY);

inline Mat bandpass(const Mat& sourceimgDFTIn, const Mat& bandpassMat)
{
	Mat sourceimgDFT = sourceimgDFTIn.clone();
	Mat filterGS = quadrantswap(bandpassMat);
	Mat filter;
	Mat filterPlanes[2] = { filterGS, filterGS };
	merge(filterPlanes, 2, filter);
	return sourceimgDFT.mul(filter);
}

Mat convolute(Mat sourceimg, Mat PSFimg);

Mat deconvolute(Mat sourceimg, Mat PSFimg);

Mat deconvoluteWiener(const Mat& sourceimg, const Mat& PSFimg);

Mat frequencyFilter(const Mat& sourceimg, const Mat& mask);
