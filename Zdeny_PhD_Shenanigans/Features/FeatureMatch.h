#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

enum FeatureType
{
	BRISK,
	ORB,
	MSER,
	FAST,
	AGAST,
	GFTT,
	KAZE,
	AKAZE
};

inline int GetFeatureTypeMatcher( FeatureType ftype )
{
	switch ( ftype )
	{
		case FeatureType::BRISK:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
		case FeatureType::ORB:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
		case FeatureType::MSER:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
		case FeatureType::FAST:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
		case FeatureType::AGAST:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
		case FeatureType::GFTT:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
		case FeatureType::KAZE:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
		case FeatureType::AKAZE:
			return DescriptorMatcher::BRUTEFORCE_HAMMING;
	}
}

inline void featureMatch( std::string path1, std::string path2, int featurecnt, FeatureType ftype )
{
	Mat img1 = imread( path1, IMREAD_GRAYSCALE );
	Mat img2 = imread( path2, IMREAD_GRAYSCALE );

	showimg( img1, "img1 source" );
	showimg( img2, "img2 source" );

	//detect the keypoints, compute the descriptors
	Ptr<Feature2D> detector;
	detector = ORB::create();
	std::vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;
	detector->detectAndCompute( img1, noArray(), keypoints1, descriptors1 );
	detector->detectAndCompute( img2, noArray(), keypoints2, descriptors2 );

	//matching descriptor vectors
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create( GetFeatureTypeMatcher( ftype ) );
	std::vector<DMatch> matches;
	matcher->match( descriptors1, descriptors2, matches );

	//get first n best matches
	std::sort( matches.begin(), matches.end(), []( DMatch a, DMatch b ) { return a.distance < b.distance; } );
	matches = std::vector<DMatch>( matches.begin(), matches.begin() + min( featurecnt, ( int )matches.size() ) );

	//calculate feature shifts
	std::vector<Point2f> shifts( matches.size() );
	for ( int i = 0; i < matches.size(); i++ )
	{
		int idx1 = matches[i].queryIdx;
		int idx2 = matches[i].trainIdx;

		shifts[i] = keypoints2[idx2].pt - keypoints1[idx1].pt;
		LOG_DEBUG( "Calculated feature shift[{}] =[{},{}]", i, shifts[i].x, shifts[i].y );
	}

	auto avgshift = mean( shifts );
	LOG_SUCC( "Calculated average feature shift=[{},{}] - angle={} deg", avgshift.x, avgshift.y, ( int )toDegrees( atan2( avgshift.y, avgshift.x ) ) );

	//draw matches
	Mat img_matches;
	drawMatches( img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar::all( -1 ), Scalar::all( -1 ), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
	resize( img_matches, img_matches, cv::Size( 0.5 * img_matches.cols, 0.5 * img_matches.rows ) );
	imshow( "Good Matches", img_matches );
}