#pragma once
#include "stdafx.h"

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

			float x = (double)rand() / RAND_MAX;

			if (x < 0.3)
			{
				noise1.at<Vec3f>(r, c)[0] = 156. / 255;
				noise1.at<Vec3f>(r, c)[1] = 50. / 255;
				noise1.at<Vec3f>(r, c)[2] = 55. / 255;
			}
			else if (x < 0.4)
			{
				noise1.at<Vec3f>(r, c)[0] = 62. / 255;
				noise1.at<Vec3f>(r, c)[1] = 226. / 255;
				noise1.at<Vec3f>(r, c)[2] = 254. / 255;
			}
			else if (x < 0.7)
			{
				noise1.at<Vec3f>(r, c)[0] = 35./255;
				noise1.at<Vec3f>(r, c)[1] = 102./255;
				noise1.at<Vec3f>(r, c)[2] = 11./255;
			}
			else if (x < 0.9)
			{
				noise1.at<Vec3f>(r, c)[0] = 0.3;
				noise1.at<Vec3f>(r, c)[1] = 0.3;
				noise1.at<Vec3f>(r, c)[2] = 0.3;
			}
			else
			{
				noise1.at<Vec3f>(r, c)[0] = 0.6;
				noise1.at<Vec3f>(r, c)[1] = 0.6;
				noise1.at<Vec3f>(r, c)[2] = 0.6;
			}
			
			// B G R
			noise2.at<Vec3f>(r, c)[0] = (double)rand() / RAND_MAX;
			noise2.at<Vec3f>(r, c)[1] = (double)rand() / RAND_MAX;
			noise2.at<Vec3f>(r, c)[2] = (double)rand() / RAND_MAX;
			
		}
	}
	return std::make_pair(noise1, noise2);
}

