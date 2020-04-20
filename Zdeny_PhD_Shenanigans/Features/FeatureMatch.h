#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

inline void featureMatch( std::string path1, std::string path2, int featurecnt )
{
	Mat img1 = imread( path1, IMREAD_GRAYSCALE );
	Mat img2 = imread( path2, IMREAD_GRAYSCALE );

	showimg( img1, "img1 source" );
	showimg( img2, "img2 source" );

	//detect the keypoints, compute the descriptors
	Ptr<AKAZE> detector = AKAZE::create();
	std::vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;
	detector->detectAndCompute( img1, noArray(), keypoints1, descriptors1 );
	detector->detectAndCompute( img2, noArray(), keypoints2, descriptors2 );

	//matching descriptor vectors
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create( DescriptorMatcher::BRUTEFORCE_HAMMING );
	std::vector<DMatch> matches;
	matcher->match( descriptors1, descriptors2, matches );

	//get first n best matches
	std::sort( matches.begin(), matches.end(), []( DMatch a, DMatch b ) { return a.distance < b.distance; } );
	matches = std::vector<DMatch>( matches.begin(), matches.begin() + featurecnt );

	//calculate feature shifts
	std::vector<Point2f> shifts( matches.size() );
	for ( int i = 0; i < matches.size(); i++ )
	{
		int idx1 = matches[i].queryIdx;
		int idx2 = matches[i].trainIdx;

		shifts[i] = keypoints1[idx1].pt - keypoints2[idx2].pt;
		LOG_DEBUG( "Calculated feature shift[{}] =[{},{}]", i, shifts[i].x, shifts[i].y );
	}

	auto avgshift = mean( shifts );
	LOG_SUCC( "Calculated average feature shift=[{},{}]", avgshift.x, avgshift.y );

	//draw matches
	Mat img_matches;
	drawMatches( img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar::all( -1 ), Scalar::all( -1 ), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
	resize( img_matches, img_matches, cv::Size( 0.5 * img_matches.cols, 0.5 * img_matches.rows ) );
	imshow( "Good Matches", img_matches );
}