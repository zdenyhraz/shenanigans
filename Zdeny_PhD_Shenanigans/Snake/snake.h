#pragma once
#include "stdafx.h"

using namespace cv;

class Map;

struct Coordinate
{
	Coordinate(unsigned x_, unsigned y_) :x(x_), y(y_) {}

	unsigned x = 0;
	unsigned y = 0;
};

class Snake
{
public:
	Snake(Map& map);

	enum Direction
	{
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

private:
	std::vector<Coordinate> m_body;
	Direction m_direction;
	bool m_gameover;

	void Grow();

	bool CheckValidMove();

	void Tick();

	void Turn(Direction direction);

};