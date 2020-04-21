#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

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
	bool distancesort;
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

inline void featureMatch( std::string path1, std::string path2, const FeatureMatchData &data )
{
	Mat img1 = imread( path1, IMREAD_GRAYSCALE );
	Mat img2 = imread( path2, IMREAD_GRAYSCALE );

	showimg( std::vector<Mat> {img1, img2}, "imgs source" );

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

	if ( data.distancesort )
	{
		//get first n smallest shifts
		std::sort( matches.begin(), matches.end(), [&]( DMatch a, DMatch b ) { return magnitude( GetFeatureMatchShift( a, keypoints1, keypoints2 ) ) < magnitude( GetFeatureMatchShift( b, keypoints1, keypoints2 ) ); } );
		matches = std::vector<DMatch>( matches.begin(), matches.begin() + min( data.matchcnt, ( int )matches.size() ) );
	}
	else
	{
		//get first n best matches
		std::sort( matches.begin(), matches.end(), []( DMatch a, DMatch b ) { return a.distance < b.distance; } );
		matches = std::vector<DMatch>( matches.begin(), matches.begin() + min( data.matchcnt, ( int )matches.size() ) );
	}

	//calculate feature shifts
	std::vector<Point2f> shifts( matches.size() );
	for ( int i = 0; i < matches.size(); i++ )
	{
		shifts[i] = GetFeatureMatchShift( matches[i], keypoints1, keypoints2 );
		LOG_DEBUG( "Calculated feature shift[{}] =[{},{}]", i, shifts[i].x, shifts[i].y );
	}

	auto avgshift = mean( shifts );
	LOG_SUCC( "Calculated average feature shift=[{},{}] => angle={} deg", avgshift.x, avgshift.y, toDegrees( atan2( -avgshift.y, avgshift.x ) ) );

	//draw matches
	Mat img_matches;
	drawMatches( img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar( 0, 255, 255 ), Scalar( 0, 255, 0 ), std::vector<char>(), DrawMatchesFlags::DEFAULT );
	resize( img_matches, img_matches, cv::Size( 0.5 * img_matches.cols, 0.5 * img_matches.rows ) );
	imshow( "Good matches", img_matches );
}