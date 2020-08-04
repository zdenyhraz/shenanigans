#pragma once
#include "stdafx.h"
#include "diffrotResults.h"

void SaveDiffrotResultsToFile( const std::string &dir, const std::string &filename, DiffrotResults *dr )
{
	LOG_STARTEND( "Saving diffrot results ...", "Diffrot results saved" );

	std::string path = dir + filename + ".diffrot";

	if ( std::filesystem::exists( path ) )
	{
		LOG_ERROR( "File {} already exists, cannot save", path );
		return;
	}

	if ( !dr->calculated )
	{
		LOG_ERROR( "Diffrot results not yet calculated!" );
		return;
	}

	FileStorage fs( path, FileStorage::WRITE );

	std::vector<double> SourceThetasavg;
	std::vector<double> SourceOmegasXavg;
	std::vector<double> SourceOmegasYavg;
	std::vector<double> SourceShiftsXavg;
	std::vector<double> SourceShiftsYavg;
	dr->GetVecs1D( SourceThetasavg, SourceOmegasXavg, SourceOmegasYavg, SourceShiftsXavg, SourceShiftsYavg );
	fs << "SourceThetasavg" << SourceThetasavg;
	fs << "SourceOmegasXavg" << SourceOmegasXavg;
	fs << "SourceOmegasYavg" << SourceOmegasYavg;
	fs << "SourceShiftsXavg" << SourceShiftsXavg;
	fs << "SourceShiftsYavg" << SourceShiftsYavg;

	std::vector<std::vector<double>> SourceShiftsX;
	std::vector<std::vector<double>> SourceShiftsY;
	dr->GetVecs2D( SourceShiftsX, SourceShiftsY );
	fs << "SourceShiftsX" << SourceShiftsX;
	fs << "SourceShiftsY" << SourceShiftsY;

	Mat SourceImage;
	Mat SourceFlowX;
	Mat SourceFlowY;
	dr->GetMats( SourceImage, SourceFlowX, SourceFlowY );
	fs << "SourceImage" << SourceImage;
	fs << "SourceFlowX" << SourceFlowX;
	fs << "SourceFlowY" << SourceFlowY;

	int SourcePics;
	int SourceStride;
	dr->GetParams( SourcePics, SourceStride );
	fs << "SourcePics" << SourcePics;
	fs << "SourceStride" << SourceStride;
}

void LoadDiffrotResultsFromFile( const std::string &path, DiffrotResults *dr )
{
	LOG_STARTEND( "Loading diffrot results ...", "Diffrot results loaded" );

	if ( !std::filesystem::exists( path ) )
	{
		LOG_ERROR( "File {} does not exist, cannot load", path );
		return;
	}

	FileStorage fs( path, FileStorage::READ );

	std::vector<double> SourceThetasavg;
	std::vector<double> SourceOmegasXavg;
	std::vector<double> SourceOmegasYavg;
	std::vector<double> SourceShiftsXavg;
	std::vector<double> SourceShiftsYavg;
	fs["SourceThetasavg"] >> SourceThetasavg;
	fs["SourceOmegasXavg"] >> SourceOmegasXavg;
	fs["SourceOmegasYavg"] >> SourceOmegasYavg;
	fs["SourceShiftsXavg"] >> SourceShiftsXavg;
	fs["SourceShiftsYavg"] >> SourceShiftsYavg;
	dr->SetVecs1DRaw( SourceThetasavg, SourceOmegasXavg, SourceOmegasYavg, SourceShiftsXavg, SourceShiftsYavg );

	std::vector<std::vector<double>> SourceShiftsX;
	std::vector<std::vector<double>> SourceShiftsY;
	fs["SourceShiftsX"] >> SourceShiftsX;
	fs["SourceShiftsY"] >> SourceShiftsY;
	dr->SetVecs2DRaw( SourceShiftsX, SourceShiftsY );

	Mat SourceImage;
	Mat SourceFlowX;
	Mat SourceFlowY;
	fs["SourceImage"] >> SourceImage;
	fs["SourceFlowX"] >> SourceFlowX;
	fs["SourceFlowY"] >> SourceFlowY;
	dr->SetMatsRaw( SourceImage, SourceFlowX, SourceFlowY );

	int SourcePics;
	int SourceStride;
	fs["SourcePics"] >> SourcePics;
	fs["SourceStride"] >> SourceStride;
	dr->SetParamsRaw( SourcePics, SourceStride );
}
