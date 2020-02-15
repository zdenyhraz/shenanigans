#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

using namespace std;
using namespace cv;

struct filterSettings
{
	double contrast;
	double brightness;
	double gamma;

	filterSettings(double contrast, double brightness, double gamma) : contrast(contrast), brightness(brightness), gamma(gamma) {}
};

Mat filterContrastBrightness(const Mat& sourceimg, double contrast, double brightness);

Mat histogramEqualize(const Mat& sourceimgIn);

Mat gammaCorrect(const Mat& sourceimgIn, double gamma);

Mat addnoise(const Mat& sourceimgIn);

void showhistogram(const Mat& sourceimgIn, int channels, int minimum = 0, int maximum = 255, std::string winname = "histogram");
