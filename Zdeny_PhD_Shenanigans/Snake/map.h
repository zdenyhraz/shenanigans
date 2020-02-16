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

	Mat Draw(Snake Bob);

	Snake::Coordinate GetFood();

	void SetFood();

private:
	Mat m_map;
	Snake::Coordinate m_food;
	void Clear();
};