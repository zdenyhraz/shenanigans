#pragma once
#include "stdafx.h"
#include "baseFunctions2d.h"

using namespace std;
using namespace cv;

std::pair<Mat, Mat> SmoothNoise(int rows, int cols)
{
	Mat noise1(rows, cols, CV_32FC3);
	Mat noise2(rows, cols, CV_32FC3);

	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			// B G R
			noise1.at<Vec3f>(r, c)[0] = 0;
			noise1.at<Vec3f>(r, c)[1] = 0;
			noise1.at<Vec3f>(r, c)[2] = 0;

			//float x = (double)rand() / RAND_MAX;
			//float x = (double)r*c/(rows*cols);
			//float x = (double)(r + c) / (rows + cols);
			float x = exp((-pow(r - rows / 2, 2) - pow(c - cols / 2, 2)) / 10 / (rows + cols));


			if (x < 0.13)
			{
				noise1.at<Vec3f>(r, c)[0] = 255. / 255;
				noise1.at<Vec3f>(r, c)[1] = 0. / 255;
				noise1.at<Vec3f>(r, c)[2] = 0. / 255;
			}
			else if (x < 0.3)
			{
				noise1.at<Vec3f>(r, c)[0] = 255. / 255;
				noise1.at<Vec3f>(r, c)[1] = 191. / 255;
				noise1.at<Vec3f>(r, c)[2] = 0. / 255;
			}
			else if (x < 0.43)
			{
				noise1.at<Vec3f>(r, c)[0] = 62. / 255;
				noise1.at<Vec3f>(r, c)[1] = 226. / 255;
				noise1.at<Vec3f>(r, c)[2] = 254. / 255;
			}
			else if (x < 0.65)
			{
				noise1.at<Vec3f>(r, c)[0] = 0./255;
				noise1.at<Vec3f>(r, c)[1] = 128./255;
				noise1.at<Vec3f>(r, c)[2] = 0./255;
			}
			else if (x < 0.87)
			{
				noise1.at<Vec3f>(r, c)[0] = 0.5;
				noise1.at<Vec3f>(r, c)[1] = 0.5;
				noise1.at<Vec3f>(r, c)[2] = 0.5;
			}
			else
			{
				noise1.at<Vec3f>(r, c)[0] = 0.88;
				noise1.at<Vec3f>(r, c)[1] = 0.88;
				noise1.at<Vec3f>(r, c)[2] = 0.88;
			}
			
			// B G R
			float f = exp((-pow(r - rows / 2, 2) / 1000 - pow(c - cols / 2, 2) / 200));
			noise2.at<Vec3f>(r, c)[0] = f;
			noise2.at<Vec3f>(r, c)[1] = f;
			noise2.at<Vec3f>(r, c)[2] = f;
			
		}
	}
	return std::make_pair(noise1, noise2);
}

