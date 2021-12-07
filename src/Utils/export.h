#pragma once
#include "Core/functionsBaseCV.h"

inline void saveMatToCsv( const std::string &path, const cv::Mat &matIn )
{
	std::ofstream listing( path, std::ios::out | std::ios::trunc );
	cv::Mat mat = matIn.clone();
	mat.convertTo( mat, CV_32F );
	for ( int r = 0; r < mat.rows; r++ )
	{
		for ( int c = 0; c < mat.cols - 1; c++ )
		{
			listing << mat.at<float>( r, c ) << ",";
		}
		listing << mat.at<float>( r, mat.cols - 1 ) << std::endl;
	}
}

inline void exportToMATLAB( const cv::Mat &Zdata, double xmin, double xmax, double ymin, double ymax, std::string path )
{
	std::ofstream listing( path, std::ios::out | std::ios::trunc );
	listing << xmin << "," << xmax << "," << ymin << "," << ymax << std::endl;
	for ( int r = 0; r < Zdata.rows; r++ )
		for ( int c = 0; c < Zdata.cols; c++ )
			listing << Zdata.at<float>( r, c ) << std::endl;
}