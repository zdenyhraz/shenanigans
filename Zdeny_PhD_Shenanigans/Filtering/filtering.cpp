#include "stdafx.h"
#include "filtering.h"

Mat filterContrastBrightness(const Mat& sourceimg, double contrast, double brightness)
{
	Mat filtered = sourceimg.clone();
	filtered.convertTo(filtered, CV_16U);
	normalize(filtered, filtered, 0, 65535, CV_MINMAX);
	for (int r = 0; r < filtered.rows; r++)
	{
		for (int c = 0; c < filtered.cols; c++)
		{
			if (filtered.channels() > 1)
			{
				filtered.at<Vec3w>(r, c)[0] = clamp(contrast * (filtered.at<Vec3w>(r, c)[0] + brightness), 0, 65535);
				filtered.at<Vec3w>(r, c)[1] = clamp(contrast * (filtered.at<Vec3w>(r, c)[1] + brightness), 0, 65535);
				filtered.at<Vec3w>(r, c)[2] = clamp(contrast * (filtered.at<Vec3w>(r, c)[2] + brightness), 0, 65535);
			}
			else
			{
				filtered.at<ushort>(r, c) = clamp(contrast * ((double)filtered.at<ushort>(r, c) + brightness), 0, 65535);
			}
		}
	}
	normalize(filtered, filtered, 0, 65535, CV_MINMAX);
	return filtered;
}

Mat histogramEqualize(const Mat& sourceimgIn)
{
	Mat sourceimg = sourceimgIn.clone();
	normalize(sourceimg, sourceimg, 0, 255, CV_MINMAX);
	sourceimg.convertTo(sourceimg, CV_8U);
	equalizeHist(sourceimg, sourceimg);
	sourceimg.convertTo(sourceimg, CV_16U);
	normalize(sourceimg, sourceimg, 0, 65535, CV_MINMAX);
	std::cout << "histogram equalized" << std::endl;
	return sourceimg;
}

Mat gammaCorrect(const Mat& sourceimgIn, double gamma)//returns CV_16UC1/3
{
	Mat sourceimg = sourceimgIn.clone();

	sourceimg.convertTo(sourceimg, CV_32F);
	normalize(sourceimg, sourceimg, 0, 65535, CV_MINMAX);
	pow(sourceimg, 1. / gamma, sourceimg);
	sourceimg.convertTo(sourceimg, CV_16U);
	normalize(sourceimg, sourceimg, 0, 65535, CV_MINMAX);

	return sourceimg;
}

Mat addnoise(const Mat& sourceimgIn)
{
	Mat sourceimg = sourceimgIn.clone();
	sourceimg.convertTo(sourceimg, CV_32F);
	Mat noised = Mat::zeros(sourceimg.rows, sourceimg.cols, CV_32F);
	randn(noised, 0, 255 / 50);
	noised = noised + sourceimg;
	normalize(noised, noised, 0, 255, CV_MINMAX);
	noised.convertTo(noised, CV_8U);
	return noised;
}

void showhistogram(const Mat& sourceimgIn, int channels, int minimum, int maximum, std::string winname)
{
	Mat sourceimg = sourceimgIn.clone();
	/// Separate the image in 3 places ( B, G and R )
	vector<Mat> bgr_planes(3);
	if (channels > 1)
		split(sourceimg, bgr_planes);
	else
	{
		bgr_planes[0] = sourceimg;
		bgr_planes[1] = sourceimg;
		bgr_planes[2] = sourceimg;
	}

	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { (float)minimum, (float)maximum };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// Draw for each channel
	int thickness = 1;
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point2i(bin_w*(i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
			Point2i(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))),
			Scalar(255, 0, 0), thickness, 8, 0);
		line(histImage, Point2i(bin_w*(i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
			Point2i(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))),
			Scalar(0, 255, 0), thickness, 8, 0);
		line(histImage, Point2i(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
			Point2i(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))),
			Scalar(0, 0, 255), thickness, 8, 0);
	}

	/// Display
	showimg(histImage, winname);

}

