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

	Mat DrawBody(Snake Bob);

private:
	Mat m_map;

	void Clear();

	std::pair<unsigned, unsigned> food;
};