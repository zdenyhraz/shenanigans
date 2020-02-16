#pragma once
#include "stdafx.h"
#include "snake.h"

using namespace cv;

class Map
{
public: 
	Map(unsigned width, unsigned height);
	
	unsigned width;
	unsigned height;

private:
	Mat map;

	void DrawBody(Snake Bob);
	
	std::pair<unsigned, unsigned> food;
};