#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"

using namespace std;
using namespace cv;

struct filterSettings
{
	double contrast;
	double brightness;
	double gamma;

	filterSettings(double contrast, double brightness, double gamma) : contrast(contrast), brightness(brightness), gamma(gamma) {}
};

Mat filterContrastBrightness(Mat& sourceimg, double contrast, double brightness);

Mat histogramEqualize(Mat& sourceimgIn);

Mat gammaCorrect(Mat& sourceimgIn, double gamma);

Mat addnoise(Mat& sourceimgIn);

void showhistogram(Mat& sourceimgIn, int channels, int minimum = 0, int maximum = 255, std::string winname = "histogram");