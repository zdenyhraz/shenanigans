#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

inline void featureMatch( std::string path1, std::string path2 )
{
	Mat img1 = loadImage( path1 );
	Mat img2 = loadImage( path2 );

	showimg( img1, "img1 source" );
	showimg( img2, "img2 source" );
}