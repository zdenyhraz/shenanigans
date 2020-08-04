#pragma once
#include "stdafx.h"
#include "diffrotResults.h"

template <typename T>
void SaveTupleToStorage( std::vector<T> data, std::vector<const char *> labels, FileStorage &fs )
{
	for ( int i = 0; i < labels.size(); i++ )
		fs << labels[i] << data[i];
}

bool CheckIfFileExists( const std::string &path )
{
	if ( !std::filesystem::exists( path ) )
	{
		LOG_ERROR( "File {} does not exist", path );
		return false;
	}
	return true;
}

bool CheckIfFileAlreadyExists( const std::string &path )
{
	if ( std::filesystem::exists( path ) )
	{
		LOG_ERROR( "File {} already exists", path );
		return true;
	}
	return false;
}

void SaveDiffrotResultsToFile( const std::string &dir, const std::string &filename, DiffrotResults *dr )
{
	if ( !dr->calculated )
	{
		LOG_ERROR( "Diffrot results not yet calculated!" );
		return;
	}

	LOG_STARTEND( "Saving diffrot results ...", "Diffrot results saved" );

	std::string path = dir + filename + ".diffrot";
	if ( CheckIfFileAlreadyExists( path ) )
		return;

	return;
	FileStorage fs( path, FileStorage::WRITE );

	auto [dataVecs1D, labelsVecs1D] = dr->GetVecs1D();
	auto [dataVecs2D, labelsVecs2D] = dr->GetVecs2D();
	auto [dataMats, labelsMats] = dr->GetMats();
	auto [dataParams, labelsParams] = dr->GetParams();

	SaveTupleToStorage( dataVecs1D, labelsVecs1D, fs );
	SaveTupleToStorage( dataVecs2D, labelsVecs2D, fs );
	SaveTupleToStorage( dataMats, labelsMats, fs );
	SaveTupleToStorage( dataParams, labelsParams, fs );
}

void LoadDiffrotResultsFromFile( const std::string &path, DiffrotResults *dr )
{
	LOG_STARTEND( "Loading diffrot results ...", "Diffrot results loaded" );
	if ( !CheckIfFileExists( path ) )
		return;

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
