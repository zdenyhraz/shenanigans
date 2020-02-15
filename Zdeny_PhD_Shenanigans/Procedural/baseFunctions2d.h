#pragma once
#include "stdafx.h"
#include "Core/constants.h"

using namespace cv;
using namespace std;

inline Mat sinian(int rows, int cols)
{
	Mat mat = Mat::zeros(rows, cols, CV_32F);
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			mat.at<float>(r, c) = sin(Constants::TwoPi*c / cols) + sin(Constants::TwoPi*r / rows);
		}
	}
	return mat;
}