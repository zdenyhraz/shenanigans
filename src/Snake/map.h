#pragma once
#include <opencv2/opencv.hpp>

#include "snake.h"

class Map
{
public: 
	Map(unsigned width, unsigned height);
	
	unsigned width;
	unsigned height;

	cv::Mat Draw(Snake Bob);

	Snake::Coordinate GetFood();

	void SetFood();

private:
	cv::Mat m_map;
	Snake::Coordinate m_food;
	void Clear();
};