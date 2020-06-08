#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fit/polyfit.h"
#include "Fit/nnfit.h"
#include "Draw/combinepics.h"
#include "Utils/export.h"

static constexpr int piccnt = 10;//number of pics
static constexpr double scale = 10;//scale for visualization
static constexpr double kmpp = 696010. / 378.3;//kilometers per pixel
static constexpr double dt = 11.88;//dt temporally adjacent pics
static constexpr double arrow_scale = 2;//size of the arrow
static constexpr double ratio_thresh = 0.7;//Lowe's ratio test
static constexpr double text_scale = scale / 4;//text scale
static constexpr double text_thickness = scale / 4;//text thickness

enum FeatureType
{
	SURF,
	BRISK,
	ORB,
	MSER,
	FAST,
	AGAST,
	GFTT,
	KAZE,
	AKAZE
};

struct FeatureMatchData
{
	FeatureType ftype;
	double thresh;
	int matchcnt;
	double magnitudeweight;
	double quanB;
	double quanT;
	std::string path1;
	std::string path2;
	std::string path;
	std::string pathout;
	int degree;
	int proxpts;
	double proxcoeff;
};

inline Point2f GetFeatureMatchShift( const DMatch &match, const std::vector<KeyPoint> &kp1, const std::vector<KeyPoint> &kp2 )
{
	return kp2[match.trainIdx].pt - kp1[match.queryIdx].pt;
}

inline std::pair<Point2f, Point2f> GetFeatureMatchPoints( const DMatch &match, const std::vector<KeyPoint> &kp1, const std::vector<KeyPoint> &kp2 )
{
	return std::make_pair( kp1[match.queryIdx].pt, kp2[match.trainIdx].pt );
}

inline int GetFeatureTypeMatcher( const FeatureMatchData &data )
{
	switch ( data.ftype )
	{
		case FeatureType::SURF:
			return DescriptorMatcher::MatcherType::BRUTEFORCE;
		case FeatureType::BRISK:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
		case FeatureType::ORB:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
		case FeatureType::MSER:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
		case FeatureType::FAST:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
		case FeatureType::AGAST:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
		case FeatureType::GFTT:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
		case FeatureType::KAZE:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
		case FeatureType::AKAZE:
			return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
	}
	return DescriptorMatcher::MatcherType::BRUTEFORCE_HAMMING;
}

inline Ptr<Feature2D> GetFeatureDetector( const FeatureMatchData &data )
{
	switch ( data.ftype )
	{
		case FeatureType::SURF:
			return xfeatures2d::SURF::create( data.thresh );
		case FeatureType::BRISK:
			return BRISK::create();
		case FeatureType::ORB:
			return ORB::create();
		case FeatureType::MSER:
			return MSER::create();
		case FeatureType::FAST:
			return ORB::create();
		case FeatureType::AGAST:
			return ORB::create();
		case FeatureType::GFTT:
			return ORB::create();
		case FeatureType::KAZE:
			return KAZE::create();
		case FeatureType::AKAZE:
			return AKAZE::create();
	}
	return ORB::create();
}

inline void exportFeaturesToCsv( const std::string &path, const std::vector<Point2f> &points, const std::vector<double> &speeds, const std::vector<double> &directions )
{
	std::string pth = path + "features.csv";
	std::ofstream csv( pth, std::ios::out | std::ios::trunc );
	csv << "X,Y,SPD,DIR" << endl;
	for ( int i = 0; i < points.size(); i++ )
	{
		csv << points[i].x << "," << points[i].y << "," << speeds[i] << "," << directions[i] << endl;
	}
	LOG_INFO( "Feature data exported to {}", pth );
}

inline std::tuple<Mat, Mat, Mat, Mat> DrawFeatureMatchArrows( const Mat &img, const std::vector<std::vector<DMatch>> &matches_all, const std::vector<std::vector<KeyPoint>> &kp1_all, const std::vector<std::vector<KeyPoint>> &kp2_all, const std::vector<std::vector<double>> &speeds_all, const FeatureMatchData &data )
{
	Mat out;
	cvtColor( img, out, COLOR_GRAY2BGR );
	resize( out, out, Size( scale * out.cols, scale * out.rows ) );

	Mat outPt = Mat::zeros( out.rows, out.cols, CV_8UC3 );

	double minspd = getQuantile( speeds_all, data.quanB );
	double maxspd = getQuantile( speeds_all, data.quanT );

	std::vector<Point2f> points;
	std::vector<double> speeds;
	std::vector<double> directions;

	for ( int pic = 0; pic < piccnt - 1; pic++ )
	{
		for ( auto &match : matches_all[pic] )
		{
			auto shift = GetFeatureMatchShift( match, kp1_all[pic], kp2_all[pic] );
			double spd = magnitude( shift ) * kmpp / dt;
			double dir = toDegrees( atan2( -shift.y, shift.x ) );

			if ( spd < minspd )
				continue;

			if ( spd > maxspd )
				continue;

			if ( dir < 0.50 * 360 )
				continue;

			if ( dir > 0.75 * 360 )
				continue;

			auto pts = GetFeatureMatchPoints( match, kp1_all[pic], kp2_all[pic] );
			Point2f arrStart = scale * pts.first;
			Point2f arrEnd = scale * pts.first + arrow_scale * ( scale * pts.second - scale * pts.first );
			Point2f textpos = ( arrStart + arrEnd ) / 2;
			Scalar clr = colorMapJet( spd, minspd, maxspd );
			textpos.x += scale * 2;
			textpos.y += scale * 4;
			arrowedLine( out, arrStart, arrEnd, clr, scale / 2 );
			drawPoint( outPt, arrStart, clr, scale, scale / 2 );
			putText( out, to_stringp( spd, 1 ), textpos, 1, text_scale, clr, text_thickness );

			points.push_back( pts.first );
			speeds.push_back( spd );
			directions.push_back( dir );
		}
	}

	exportFeaturesToCsv( data.pathout, points, speeds, directions );

	Mat outSwnn = wnnfit( points, speeds, 0, img.cols - 1, 0, img.rows - 1, img.cols, img.rows, data.proxpts, data.proxcoeff );
	Mat outDwnn = wnnfit( points, directions, 0, img.cols - 1, 0, img.rows - 1, img.cols, img.rows, data.proxpts, data.proxcoeff );

	return std::make_tuple( out, outPt, outSwnn, outDwnn );
}

inline void featureMatch( const FeatureMatchData &data )
{
	Mat img_base = imread( data.path + "0.PNG", IMREAD_GRAYSCALE );
	Mat img_base_ups;
	resize( img_base, img_base_ups, Size( scale * img_base.cols, scale * img_base.rows ) );
	std::vector<std::vector<DMatch>> matches_all( piccnt - 1 );
	std::vector<std::vector<KeyPoint>> keypoints1_all( piccnt - 1 );
	std::vector<std::vector<KeyPoint>> keypoints2_all( piccnt - 1 );
	std::vector<std::vector<double>> speeds_all( piccnt - 1 );

	#pragma omp parallel for
	for ( int pic = 0; pic < piccnt - 1; pic++ )
	{
		std::string path1 = data.path + to_string( pic ) + ".PNG";
		std::string path2 = data.path + to_string( pic + 1 ) + ".PNG";

		LOG_DEBUG( "Matching imagees {} - {}", path1, path2 );

		Mat img1 = imread( path1, IMREAD_GRAYSCALE );
		Mat img2 = imread( path2, IMREAD_GRAYSCALE );

		//saveMatToCsv( data.pathout + "image.csv", img1 );

		//detect the keypoints, compute the descriptors
		Ptr<Feature2D> detector = GetFeatureDetector( data );
		std::vector<KeyPoint> keypoints1, keypoints2;
		Mat descriptors1, descriptors2;
		detector->detectAndCompute( img1, noArray(), keypoints1, descriptors1 );
		detector->detectAndCompute( img2, noArray(), keypoints2, descriptors2 );
		keypoints1_all[pic] = keypoints1;
		keypoints2_all[pic] = keypoints2 ;

		//matching descriptor vectors
		Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create( ( DescriptorMatcher::MatcherType )GetFeatureTypeMatcher( data ) );
		std::vector< std::vector<DMatch>> knn_matches;
		matcher->knnMatch( descriptors1, descriptors2, knn_matches, 2 );

		//filter matches using the Lowe's ratio test
		std::vector<DMatch> matches;
		for ( size_t i = 0; i < knn_matches.size(); i++ )
			if ( knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance )
				matches.push_back( knn_matches[i][0] );

		//get first n smallest shifts & best fits
		std::sort( matches.begin(), matches.end(), [&]( DMatch a, DMatch b ) { return ( data.magnitudeweight * magnitude( GetFeatureMatchShift( a, keypoints1, keypoints2 ) ) + a.distance ) < ( data.magnitudeweight * magnitude( GetFeatureMatchShift( b, keypoints1, keypoints2 ) ) + b.distance ); } );
		matches = std::vector<DMatch>( matches.begin(), matches.begin() + min( data.matchcnt, ( int )matches.size() ) );
		matches_all[pic] = matches;

		//calculate feature shifts
		std::vector<Point2f> shifts( matches.size() );
		std::vector<double> speeds( matches.size() );
		for ( int i = 0; i < matches.size(); i++ )
		{
			shifts[i] = GetFeatureMatchShift( matches[i], keypoints1, keypoints2 );
			speeds[i] = magnitude( shifts[i] ) * kmpp / dt;
			LOG_DEBUG( "Calculated feature shift[{}] =[{},{}], magn={}, wmagn={}, descdist={}, spd={} km/s", i, shifts[i].x, shifts[i].y, magnitude( shifts[i] ), data.magnitudeweight * magnitude( shifts[i] ), matches[i].distance, speeds[i] );
		}
		speeds_all[pic] = speeds;

		if ( 0 )
		{
			//draw matches
			Mat img_matches;
			drawMatches( img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar( 0, 255, 255 ), Scalar( 0, 255, 0 ), std::vector<char>(), DrawMatchesFlags::DEFAULT );
			showimg( img_matches, "Good matches" );
		}
	}

	//draw arrows
	auto mats = DrawFeatureMatchArrows( img_base, matches_all, keypoints1_all, keypoints2_all, speeds_all, data );
	showimg( std::get<0>( mats ), "Match arrows", false, 0, 1, 1200 );
	showimg( std::get<1>( mats ), "Match points", false, 0, 1 );
	showimg( std::get<2>( mats ), "Velocity surface Mwnn", true );
}

