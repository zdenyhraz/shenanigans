#pragma once
#include "stdafx.h"
#include "Core/constants.h"

using namespace cv;
using namespace std;
using namespace Constants;

inline Mat sinian(int rows, int cols, double rowsFreq, double colsFreq, double rowsShift, double colsShift)
{
	Mat mat = Mat::zeros(rows, cols, CV_32F);
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			mat.at<float>(r, c) = sin(colsFreq * TwoPi * ((double)c / (cols - 1) + colsShift)) + sin(rowsFreq * TwoPi * ((double)r / (rows - 1) + rowsShift));
		}
	}
	return mat;
}

inline Mat gaussian(int rows, int cols, double rowsSigma, double colsSigma, double rowsShift, double colsShift)
{
	Mat mat = Mat::zeros(rows, cols, CV_32F);
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			mat.at<float>(r, c) = exp(-pow((double)c / (cols - 1) - colsShift, 2) / colsSigma - pow((double)r / (rows - 1) - rowsShift, 2) / rowsSigma);
		}
	}
	return mat;
}

inline Mat procedural(int rows, int cols)
{
	int N = 3;
	Mat mat = Mat::zeros(rows, cols, CV_32F);

	for (int i = 0; i < N; i++)
	{
		mat += pow(N - i, 2)*sinian(rows, cols, i + 1, i + 1, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX).mul(gaussian(rows, cols, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX));
	}
	normalize(mat, mat, 0, 1, CV_MINMAX);
	return mat;
}