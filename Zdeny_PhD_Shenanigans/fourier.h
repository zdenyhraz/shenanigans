#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"

using namespace std;
using namespace cv;

Mat fourier(Mat& sourceimgIn);

Mat fourierinv(Mat& realIn, Mat& imagIn);

Mat quadrantswap(Mat& sourceimgDFT);

void showfourier(Mat& DFTimgIn, bool logar = true, bool expon = false, std::string magnwindowname = "FFTmagn", std::string phasewindowname = "FFTphase");

Mat gaussian(int rows, int cols, double stdevYmult, double stdevXmult);

Mat laplacian(int rows, int cols, double stdevYmult, double stdevXmult);

Mat bandpassian(int rows, int cols, double stdevLmult, double stdevHmult);

Mat edgemask(int rows, int cols);

Mat sinian(int rows, int cols, double frequencyX, double frequencyY);

Mat bandpass(const Mat& sourceimgDFTIn, double stdevG, double stdevL, Mat* bandpassMat = nullptr);

Mat convolute(Mat sourceimg, Mat PSFimg);

Mat deconvolute(Mat sourceimg, Mat PSFimg);

Mat deconvoluteWiener(Mat& sourceimg, Mat& PSFimg);

Mat frequencyFilter(Mat& sourceimg, Mat& mask);