#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

static constexpr double scale = 10;//scale for visualization
static constexpr double kmpp = 696010. / 378.3;//kilometers per pixel
static constexpr double dt = 11.88;//dt temporally adjacent pics
static constexpr double arrowscale = 10;//size of the arrow

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
	bool matchall;
	std::string path1;
	std::string path2;
	std::string path;
};

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
			return SURF::create( data.thresh );
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

inline Mat DrawFeatureMatchArrows( const Mat &img, const std::vector<DMatch> &matches, const std::vector<KeyPoint> &kp1, const std::vector<KeyPoint> &kp2, const std::vector<double> &speeds, const FeatureMatchData &data )
{
	Mat out;
	cvtColor( img, out, COLOR_GRAY2BGR );
	resize( out, out, Size( scale * out.cols, scale * out.rows ) );

	double minspd = getQuantile( speeds, data.quanB );
	double maxspd = getQuantile( speeds, data.quanT );

	for ( auto &match : matches )
	{
		auto shift = GetFeatureMatchShift( match, kp1, kp2 );
		double spd = magnitude( shift ) * kmpp / dt;
		double dir = toDegrees( atan2( -shift.y, shift.x ) );

		if ( spd < minspd )
			continue;

		if ( spd > maxspd )
			continue;

		if ( dir > 0.8 * 360 )
			continue;

		if ( dir < 0.5 * 360 )
			continue;

		auto pts = GetFeatureMatchPoints( match, kp1, kp2 );
		Point2f arrStart = scale * pts.first;
		Point2f arrEnd = scale * pts.first + arrowscale * ( scale * pts.second - scale * pts.first );
		Scalar clr = colorMapJet( spd, minspd, maxspd );
		//Scalar clr = colorMapJet( spd, vectorMin( speeds ), vectorMax( speeds ) );

		arrowedLine( out, arrStart, arrEnd, clr, scale / 2 );

		Point2f textpos = ( arrStart + arrEnd ) / 2;
		textpos.x += scale * 2;
		textpos.y += scale * 7;
		putText( out, to_stringp( spd, 1 ), textpos, 1, scale / 2, clr, scale / 3 );
	}
	return out;
}

inline void featureMatch( const FeatureMatchData &data )
{
	Mat img1 = imread( data.path1, IMREAD_GRAYSCALE );
	Mat img2 = imread( data.path2, IMREAD_GRAYSCALE );
	//showimg( std::vector<Mat> {img1, img2}, "imgs source" );

	//detect the keypoints, compute the descriptors
	Ptr<Feature2D> detector = GetFeatureDetector( data );
	std::vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;
	detector->detectAndCompute( img1, noArray(), keypoints1, descriptors1 );
	detector->detectAndCompute( img2, noArray(), keypoints2, descriptors2 );

	//matching descriptor vectors
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create( ( DescriptorMatcher::MatcherType )GetFeatureTypeMatcher( data ) );
	std::vector< std::vector<DMatch>> knn_matches;
	matcher->knnMatch( descriptors1, descriptors2, knn_matches, 2 );

	//filter matches using the Lowe's ratio test
	const double ratio_thresh = 0.7;
	std::vector<DMatch> matches;
	for ( size_t i = 0; i < knn_matches.size(); i++ )
	{
		if ( knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance )
		{
			matches.push_back( knn_matches[i][0] );
		}
	}

	//get first n smallest shifts & best fits
	std::sort( matches.begin(), matches.end(), [&]( DMatch a, DMatch b ) { return ( data.magnitudeweight * magnitude( GetFeatureMatchShift( a, keypoints1, keypoints2 ) ) + a.distance ) < ( data.magnitudeweight * magnitude( GetFeatureMatchShift( b, keypoints1, keypoints2 ) ) + b.distance ); } );
	matches = std::vector<DMatch>( matches.begin(), matches.begin() + min( data.matchcnt, ( int )matches.size() ) );

	//calculate feature shifts
	std::vector<Point2f> shifts( matches.size() );
	std::vector<double> speeds( matches.size() );
	for ( int i = 0; i < matches.size(); i++ )
	{
		shifts[i] = GetFeatureMatchShift( matches[i], keypoints1, keypoints2 );
		speeds[i] = magnitude( shifts[i] ) * kmpp / dt;
		LOG_DEBUG( "Calculated feature shift[{}] =[{},{}], magn={}, wmagn={}, descdist={}, spd={} km/s", i, shifts[i].x, shifts[i].y, magnitude( shifts[i] ), data.magnitudeweight * magnitude( shifts[i] ), matches[i].distance, speeds[i] );
	}

	auto avgshift = mean( shifts );
	LOG_SUCC( "Calculated average feature shift=[{},{}] => average angle={} deg", avgshift.x, avgshift.y, toDegrees( atan2( -avgshift.y, avgshift.x ) ) );

	//draw matches
	Mat img_matches;
	drawMatches( img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar( 0, 255, 255 ), Scalar( 0, 255, 0 ), std::vector<char>(), DrawMatchesFlags::DEFAULT );
	//showimg( img_matches, "Good matches" );

	//draw arrows
	Mat img_arrows = DrawFeatureMatchArrows( img1, matches, keypoints1, keypoints2, speeds, data );
	showimg( img_arrows, "Match arrows", false, 0, 1, 1200 );
}

