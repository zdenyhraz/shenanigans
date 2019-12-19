#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"

using namespace std;
using namespace cv;

Mat fourier(const Mat& sourceimgIn);

Mat fourierinv(const Mat& realIn, const Mat& imagIn);

Mat quadrantswap(const Mat& sourceimgDFT);

void showfourier(const Mat& DFTimgIn, bool logar = true, bool expon = false, std::string magnwindowname = "FFTmagn", std::string phasewindowname = "FFTphase");

Mat gaussian(int rows, int cols, double stdevYmult, double stdevXmult);

Mat laplacian(int rows, int cols, double stdevYmult, double stdevXmult);

Mat bandpassian(int rows, int cols, double stdevLmult, double stdevHmult);

Mat edgemask(int rows, int cols);

Mat sinian(int rows, int cols, double frequencyX, double frequencyY);

Mat bandpass(const Mat& sourceimgDFTIn, double stdevG, double stdevL, const Mat& bandpassMat);

Mat convolute(Mat sourceimg, Mat PSFimg);

Mat deconvolute(Mat sourceimg, Mat PSFimg);

Mat deconvoluteWiener(const Mat& sourceimg, const Mat& PSFimg);

Mat frequencyFilter(const Mat& sourceimg, const Mat& mask);
