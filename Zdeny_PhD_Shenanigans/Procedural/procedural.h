#pragma once
#include "stdafx.h"
#include "baseFunctions2d.h"

using namespace std;
using namespace cv;
using namespace Procedural;

inline Mat procedural(int rows, int cols)
{
	int N = 200;
	Mat mat = Mat::zeros(rows, cols, CV_32F);

	for (int i = 0; i < N; i++)
	{
		float cx = 0.02*(float)rand() / RAND_MAX;
		float cy = 0.02*(float)rand() / RAND_MAX;
		float ratio = abs(cx) / abs(cy);

		if (ratio > 5 || ratio < 1. / 5)
			continue;

		//mat += pow(N - i, 1)*sinian(rows, cols, i + 1, i + 1, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX).mul(gaussian(rows, cols, 0.002*(float)rand() / RAND_MAX, 0.002*(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX));
		//mat += sinian(rows, cols, i + 1, i + 1, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX).mul(gaussian(rows, cols, 0.002*(float)rand() / RAND_MAX, 0.002*(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX));
		mat += (float)rand() / RAND_MAX * gaussian(rows, cols, cx, cy, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
	}
	normalize(mat, mat, 0, 1, CV_MINMAX);
	return mat;
}

inline Mat colorlandscape(const Mat& heightmap)
{
	Mat mat = Mat::zeros(heightmap.rows, heightmap.cols, CV_32FC3);

	for (int r = 0; r < heightmap.rows; r++)
	{
		for (int c = 0; c < heightmap.cols; c++)
		{
			auto& x = heightmap.at<float>(r, c);

			if (x < 0.13)
			{
				//deep water
				mat.at<Vec3f>(r, c)[0] = 255. / 255;
				mat.at<Vec3f>(r, c)[1] = 0. / 255;
				mat.at<Vec3f>(r, c)[2] = 0. / 255;
			}
			else if (x < 0.3)
			{
				//shallow water
				mat.at<Vec3f>(r, c)[0] = 255. / 255;
				mat.at<Vec3f>(r, c)[1] = 191. / 255;
				mat.at<Vec3f>(r, c)[2] = 0. / 255;
			}
			else if (x < 0.43)
			{
				//sandy beaches
				mat.at<Vec3f>(r, c)[0] = 62. / 255;
				mat.at<Vec3f>(r, c)[1] = 226. / 255;
				mat.at<Vec3f>(r, c)[2] = 254. / 255;
			}
			else if (x < 0.65)
			{
				//grass lands
				mat.at<Vec3f>(r, c)[0] = 0. / 255;
				mat.at<Vec3f>(r, c)[1] = 128. / 255;
				mat.at<Vec3f>(r, c)[2] = 0. / 255;
			}
			else if (x < 0.87)
			{
				//rocky hills
				mat.at<Vec3f>(r, c)[0] = 0.5;
				mat.at<Vec3f>(r, c)[1] = 0.5;
				mat.at<Vec3f>(r, c)[2] = 0.5;
			}
			else
			{
				//snow
				mat.at<Vec3f>(r, c)[0] = 0.88;
				mat.at<Vec3f>(r, c)[1] = 0.88;
				mat.at<Vec3f>(r, c)[2] = 0.88;
			}
		}
	}
	return mat;
}

