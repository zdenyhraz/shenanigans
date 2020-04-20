#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"

static constexpr int SwindPicCnt = 10;

inline std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateLinearSwindFlow( const IPCsettings &set, std::string path, double SwindCropFocusX, double SwindCropFocusY )
{
	//load pics
	std::vector<Mat> pics( SwindPicCnt );
	for ( int iterPic = 0; iterPic < SwindPicCnt; iterPic++ )
	{
		pics[iterPic] = imread( path + "0" + to_string( iterPic + 1 ) + "_calib.PNG", IMREAD_ANYDEPTH );
		pics[iterPic] = roicrop( pics[iterPic], SwindCropFocusX * pics[iterPic].cols, SwindCropFocusY * pics[iterPic].rows, set.getcols(), set.getrows() );
		//saveimg( path + "cropped2//crop" + to_string( iterPic ) + ".PNG", pics[iterPic], false, cv::Size2i( 5 * pics[iterPic].cols, 5 * pics[iterPic].rows ) );
	}

	//calculate shifts
	auto shiftsX = zerovect( SwindPicCnt );
	auto shiftsY = zerovect( SwindPicCnt );
	auto indices = iota( 0, SwindPicCnt );

	for ( int iterPic = 0; iterPic < SwindPicCnt; iterPic++ )
	{
		auto shift = phasecorrel( pics[0], pics[iterPic], set );
		shiftsX[iterPic] = shift.x;
		shiftsY[iterPic] = shift.y;
		indices[iterPic] = iterPic;
	}
	for ( int iterPic = 0; iterPic < 3; iterPic++ ) //starting things merge with zero peak - omit
	{
		//shiftsX[iterPic] = 0;
		//shiftsY[iterPic] = 0;
	}
	return std::make_tuple( shiftsX, shiftsY, indices );
}

inline std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> calculateConstantSwindFlow( const IPCsettings &set, std::string path, double SwindCropFocusX, double SwindCropFocusY )
{
	//load pics
	std::vector<Mat> pics( SwindPicCnt );
	for ( int iterPic = 0; iterPic < SwindPicCnt; iterPic++ )
	{
		pics[iterPic] = imread( path + "0" + to_string( iterPic + 1 ) + "_calib.PNG", IMREAD_ANYDEPTH );
		pics[iterPic] = roicrop( pics[iterPic], SwindCropFocusX * pics[iterPic].cols, SwindCropFocusY * pics[iterPic].rows, set.getcols(), set.getrows() );
		//saveimg(path + "cropped//crop" + to_string(iterPic) + ".PNG", pics[iterPic], false, cv::Size2i(2000, 2000));
	}

	//calculate shifts
	auto shiftsX = zerovect( SwindPicCnt - 1 );
	auto shiftsY = zerovect( SwindPicCnt - 1 );
	auto indices = iota( 1, SwindPicCnt - 1 );

	for ( int iterPic = 0; iterPic < SwindPicCnt - 1; iterPic++ )
	{
		auto shift = phasecorrel( pics[iterPic], pics[iterPic + 1], set );
		shiftsX[iterPic] = shift.x;
		shiftsY[iterPic] = shift.y;
		indices[iterPic] = iterPic;
	}

	return std::make_tuple( shiftsX, shiftsY, indices );
}
