#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fourier/fourier.h"
#include "Astrophysics/FITS.h"
#include "Filtering/filtering.h"
#include "Log/logger.h"

using namespace std;
using namespace cv;

static constexpr int maxPCit = 10;
static constexpr double loglimit = 5;

class IPCsettings
{
private:
	double stdevLmultiplier = 5;
	double stdevHmultiplier = 100;
	int rows = 0;
	int cols = 0;
public:
	double L2size = 17;
	double L1ratio = 0.35;
	int UC = 31;
	double epsilon = 0;
	bool interpolate = 1;
	bool applyWindow = 1;
	bool applyBandpass = 1;
	bool subpixel = 1;
	bool crossCorrel = 0;
	bool speak = false;
	bool save = false;
	double minimalShift = 0;
	bool logar = false;
	Mat bandpass;
	Mat window;
	string auxdir = "D:\\MainOutput\\diffrot\\auxdir\\";

	IPCsettings( int Rows, int Cols, double StdevLmultiplier, double StdevHmultiplier ) : rows( Rows ), cols( Cols ), stdevLmultiplier( StdevLmultiplier ),
		stdevHmultiplier( StdevHmultiplier )
	{
		bandpass = bandpassian( rows, cols, stdevLmultiplier, stdevHmultiplier );
		window = edgemask( rows, cols );
	}

	void setBandpassParameters( double StdevLmultiplier, double StdevHmultiplier )
	{
		stdevLmultiplier = StdevLmultiplier;
		stdevHmultiplier = StdevHmultiplier;
		bandpass = bandpassian( rows, cols, stdevLmultiplier, stdevHmultiplier );
	}

	void setSize( int Rows, int Cols )
	{
		rows = Rows;
		cols = Cols;
		bandpass = bandpassian( rows, cols, stdevLmultiplier, stdevHmultiplier );
		window = edgemask( rows, cols );
	}

	const int &getrows() const
	{
		return rows;
	}

	const int &getcols() const
	{
		return cols;
	}

	const double &getL() const
	{
		return stdevLmultiplier;
	}

	const double &getH() const
	{
		return stdevHmultiplier;
	}

	const Mat &getWindow() const
	{
		return window;
	}

	const Mat &getBandpass() const
	{
		return bandpass;
	}
};

inline Point2f ipccore( Mat &&sourceimg1, Mat &&sourceimg2, const IPCsettings &set, bool forceshow = false )
{
	sourceimg1.convertTo( sourceimg1, CV_32F, 1. / 65535 );
	sourceimg2.convertTo( sourceimg2, CV_32F, 1. / 65535 );

	std::vector<Mat> showMatsGRS; // grayscale
	std::vector<Mat> showMatsCLR; // color

	if ( set.applyWindow )
	{
		multiply( sourceimg1, set.window, sourceimg1 );
		multiply( sourceimg2, set.window, sourceimg2 );
	}

	if ( set.speak || forceshow )
	{
		showMatsGRS.push_back( sourceimg1 );
		showMatsGRS.push_back( sourceimg2 );
		if ( set.applyBandpass )
			showMatsCLR.push_back( set.bandpass );
		if ( set.applyWindow )
			showMatsCLR.push_back( set.window );
	}

	if ( set.save )
	{
		saveimg( set.auxdir + "ipc_image1.png", sourceimg1 );
		saveimg( set.auxdir + "ipc_image2.png", sourceimg2 );
	}

	Point2f output;
	Mat L3;

	Mat DFT1 = fourier( std::move( sourceimg1 ) );
	Mat DFT2 = fourier( std::move( sourceimg2 ) );

	Mat planes1[2];
	Mat planes2[2];
	Mat CrossPowerPlanes[2];

	split( DFT1, planes1 );
	split( DFT2, planes2 );

	planes1[1] *= -1;//complex conjugate of second pic
	CrossPowerPlanes[0] = planes1[0].mul( planes2[0] ) - planes1[1].mul( planes2[1] ); //pointwise multiplications real
	CrossPowerPlanes[1] = planes1[0].mul( planes2[1] ) + planes1[1].mul( planes2[0] ); //imag

	if ( !set.crossCorrel )
	{
		Mat magnre, magnim;
		pow( CrossPowerPlanes[0], 2, magnre );
		pow( CrossPowerPlanes[1], 2, magnim );
		Mat normalizationdenominator = magnre + magnim;
		sqrt( normalizationdenominator, normalizationdenominator );
		CrossPowerPlanes[0] /= ( normalizationdenominator + set.epsilon );
		CrossPowerPlanes[1] /= ( normalizationdenominator + set.epsilon );
	}

	Mat CrossPower;
	merge( CrossPowerPlanes, 2, CrossPower );

	if ( set.applyBandpass )
		CrossPower = bandpass( CrossPower, set.bandpass );

	if ( 0 ) //CORRECT - complex magnitude - input can be real or complex whatever
	{
		Mat L3complex;
		dft( CrossPower, L3complex, DFT_INVERSE + DFT_SCALE );
		Mat L3planes[2];
		split( L3complex, L3planes );

		if ( set.speak )
		{
			auto minmaxR = minMaxMat( L3planes[0] );
			auto minmaxI = minMaxMat( L3planes[1] );
			LOG_DEBUG( "L3 real min/max: {}/{}", std::get<0>( minmaxR ), std::get<1>( minmaxR ) );
			LOG_DEBUG( "L3 imag min/max: {}/{}", std::get<0>( minmaxI ), std::get<1>( minmaxI ) );
		}

		magnitude( L3planes[0], L3planes[1], L3 );
	}
	else//real only (assume pure real input)
	{
		dft( CrossPower, L3, DFT_INVERSE + DFT_SCALE + DFT_REAL_OUTPUT );
		if ( set.speak )
		{
			auto minmax = minMaxMat( L3 );
			LOG_DEBUG( "L3 real min/max: {}/{}", std::get<0>( minmax ), std::get<1>( minmax ) );
		}
	}

	if ( set.logar )
	{
		normalize( L3, L3, 1, loglimit, NORM_MINMAX );
		log( L3, L3 );
	}

	L3 = quadrantswap( L3 );
	normalize( L3, L3, 0, 1, NORM_MINMAX );

	if ( set.minimalShift )
		L3 = L3.mul( 1 - kirkl( L3.rows, L3.cols, set.minimalShift ) );

	Point2i L3peak;
	double maxR;
	minMaxLoc( L3, nullptr, &maxR, nullptr, &L3peak );
	Point2f L3mid( L3.cols / 2, L3.rows / 2 );
	Point2f imageshift_PIXEL( L3peak.x - L3mid.x, L3peak.y - L3mid.y );
	if ( set.speak || forceshow )
	{
		Mat L3v;
		resize( L3, L3v, cv::Size( 2000, 2000 ), 0, 0, INTER_NEAREST );
		showMatsCLR.push_back( crosshair( L3v, Point2f( round( ( float )( L3peak.x ) * 2000. / ( float )L3.cols ), round( ( float )( L3peak.y ) * 2000. / ( float )L3.rows ) ) ) );
		if ( set.save )
		{
			auto L3vs = applyQuantileColorMap( L3v );
			saveimg( set.auxdir + "ipc_L3.png", L3vs );
		}
	}
	if ( set.speak )
	{
		LOG_DEBUG( "Phase correlation max: {} at location: {}", maxR, to_string( L3peak ) );
		LOG_DEBUG( "Calculated shift with pixel accuracy: {} pixels", to_string( imageshift_PIXEL ) );
	}
	output = imageshift_PIXEL;

	if ( set.subpixel )
	{
		bool converged = false;
		int L2size = set.L2size;
		if ( !( L2size % 2 ) )
			L2size++; //odd!+
		while ( !converged )
		{
			if ( ( ( L3peak.x - L2size / 2 ) < 0 ) || ( ( L3peak.y - L2size / 2 ) < 0 ) || ( ( L3peak.x + L2size / 2 ) >= L3.cols ) || ( ( L3peak.y + L2size / 2 ) >= L3.rows ) )
			{
				LOG_ERROR( "Degenerate peak (Imgsize=[{},{}],L3peak=[{},{}],L2size=[{},{}]) - results might be inaccurate, reducing L2size from {} to {} ", L3.cols, L3.rows, L3peak.x, L3peak.y, L2size, L2size, L2size, L2size - 2 );
				L2size -= 2;
				if ( L2size < 3 )
				{
					LOG_ERROR( "Completely degenerate peak, returning just with pixel accuracy" );
					break;
				}
			}
			else
			{
				Point2f imageshift_SUBPIXEL;
				Mat L2 = roicrop( L3, L3peak.x, L3peak.y, L2size, L2size );
				Mat L2U;

				if ( set.interpolate )
					resize( L2, L2U, L2.size()*set.UC, 0, 0, INTER_LINEAR );
				else
					resize( L2, L2U, L2.size()*set.UC, 0, 0, INTER_NEAREST );

				Point2f L2mid( L2.cols / 2, L2.rows / 2 );
				Point2f L2Umid( L2U.cols / 2, L2U.rows / 2 );
				Point2f L2Upeak = L2Umid;

				if ( set.speak || forceshow )
				{
					Mat L2Uv;
					resize( L2U, L2Uv, cv::Size( 2000, 2000 ), 0, 0, INTER_LINEAR );
					showMatsCLR.push_back( crosshair( L2Uv, L2Umid * 2000 / L2U.cols ) );
					if ( set.save )
					{
						auto L2Uvs = applyQuantileColorMap( crosshair( L2Uv, L2Umid * 2000 / L2U.cols ) );
						saveimg( set.auxdir + "ipc_L2.png", L2Uvs );
					}
				}
				if ( set.speak )
				{
					LOG_DEBUG( "L2Upeak location before iterations: {}", to_string( L2Upeak ) );
					LOG_DEBUG( "L2Upeak location before iterations findCentroid double: {}", to_string( findCentroid( L2U ) ) );
				}

				int L1size = std::round( ( float )L2U.cols * set.L1ratio );
				if ( !( L1size % 2 ) )
					L1size++; //odd!+

				Mat L1 = kirklcrop( L2U, L2Upeak.x, L2Upeak.y, L1size );
				Point2f L1mid( L1.cols / 2, L1.rows / 2 );
				imageshift_SUBPIXEL = ( Point2f )L3peak - L3mid + findCentroid( L1 ) - L1mid;

				for ( int i = 0; i < maxPCit; i++ )
				{
					if ( set.speak )
						LOG_DEBUG( "======= iteration {} =======", i + 1 );

					L1 = kirklcrop( L2U, L2Upeak.x, L2Upeak.y, L1size );
					Point2f L1peak = findCentroid( L1 );

					if ( set.speak )
						LOG_DEBUG( "L1peak: {}", to_string( L1peak ) );

					L2Upeak.x += round( L1peak.x - L1mid.x );
					L2Upeak.y += round( L1peak.y - L1mid.y );
					if ( ( L2Upeak.x > ( L2U.cols - L1mid.x - 1 ) ) || ( L2Upeak.y > ( L2U.rows - L1mid.y - 1 ) ) || ( L2Upeak.x < ( L1mid.x + 1 ) ) || ( L2Upeak.y < ( L1mid.y + 1 ) ) )
					{
						LOG_ERROR( "IPC out of bounds - centroid diverged, reducing L2size from {} to {} ", L2size, L2size - 2 );
						L2size += -2;
						break;
					}
					if ( set.speak )
					{
						LOG_DEBUG( "L1peak findCentroid double delta: {}/{}", to_string( findCentroid( L1 ).x - L1mid.x ), to_string( findCentroid( L1 ).y - L1mid.y ) );
						LOG_DEBUG( "Resulting L2Upeak in this iteration: {}", to_string( L2Upeak ) );
					}
					if ( ( abs( L1peak.x - L1mid.x ) < 0.5 ) && ( abs( L1peak.y - L1mid.y ) < 0.5 ) )
					{
						L1 = kirklcrop( L2U, L2Upeak.x, L2Upeak.y, L1size );
						if ( set.speak )
						{
							LOG_DEBUG( "Iterative phase correlation accuracy reached, L2size: {}, L2Upeak: " + to_string( L2Upeak ), L2size );
						}
						if ( set.speak || forceshow )
						{
							Mat L1v;
							resize( L1, L1v, cv::Size( 2000, 2000 ), 0, 0, INTER_LINEAR );
							showMatsCLR.push_back( crosshair( L1v, L1mid * 2000 / L1.cols ) );
							if ( set.save )
							{
								auto L1vs = applyQuantileColorMap( crosshair( L1v, L1mid * 2000 / L1.cols ) );
								saveimg( set.auxdir + "ipc_L1.png", L1vs );
							}
						}
						converged = true;
						break;
					}
					else if ( i == maxPCit - 1 )
					{
						if ( set.speak )
							LOG_DEBUG( "IPC centroid oscilated, reducing L2size from {} to {} ", L2size, L2size - 2 );

						L2size += -2;
					}
				}

				if ( converged )
				{
					imageshift_SUBPIXEL.x = ( float )L3peak.x - ( float )L3mid.x + 1.0 / ( float )set.UC * ( ( float )L2Upeak.x - ( float )L2Umid.x + findCentroid( L1 ).x - ( float )L1mid.x ); //image shift in L3 - final
					imageshift_SUBPIXEL.y = ( float )L3peak.y - ( float )L3mid.y + 1.0 / ( float )set.UC * ( ( float )L2Upeak.y - ( float )L2Umid.y + findCentroid( L1 ).y - ( float )L1mid.y ); //image shift in L3 - final

					if ( set.speak )
					{
						LOG_DEBUG( "L3 size: " + to_string( L3.cols ) + ", L2 size: " + to_string( L2.cols ) + ", L2U size: " + to_string( L2U.cols ) + ", L1 size: " + to_string( L1.cols ) );
						LOG_DEBUG( "Upsample pixel accuracy theoretical maximum: " + to_string( 1.0 / ( float )set.UC ) );
						LOG_DEBUG( "Calculated shift with pixel accuracy: " + to_string( imageshift_PIXEL ) + " pixels " );
						LOG_SUCC( "Calculated shift with subpixel accuracy1: " + to_string( imageshift_SUBPIXEL ) + " pixels " );
					}
					output = imageshift_SUBPIXEL;
				}
				else if ( L2size < 3 )
				{
					LOG_ERROR( "IPC centroid did not converge with any L2size" );
					break;
				}
			}
		}
	}

	if ( set.speak || forceshow )
	{
		showimg( showMatsGRS, "IPC input", false );
		showimg( showMatsCLR, "IPC pipeline", true );
	}

	return output;
}

inline Point2f phasecorrel( const Mat &sourceimg1In, const Mat &sourceimg2In, const IPCsettings &set, bool forceshow = false )
{
	Mat sourceimg1 = sourceimg1In.clone();
	Mat sourceimg2 = sourceimg2In.clone();

	return ipccore( std::move( sourceimg1 ), std::move( sourceimg2 ), set, forceshow );
}

inline Point2f phasecorrel( Mat &&sourceimg1, Mat &&sourceimg2, const IPCsettings &set, bool forceshow = false )
{
	return ipccore( std::move( sourceimg1 ), std::move( sourceimg2 ), set, forceshow );
}

