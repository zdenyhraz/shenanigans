#pragma once
#include "snake.h"
#include "map.h"

using namespace cv;


Snake::Snake(Map& map)
{
	//game just started
	m_gameover = false;
	m_body.reserve(3 * max(map.width, map.height));
	
	//3 units long snake
	m_body.push_back(Coordinate(map.width / 2, map.height / 2 - 1));
	m_body.push_back(Coordinate(map.width / 2, map.height / 2));
	m_body.push_back(Coordinate(map.width / 2, map.height / 2 + 1));

	//heading up
	m_direction = UP;
}

void Snake::Grow()
{
}

bool Snake::CheckValidMove()
{
}

void Snake::Tick()
{
	if (m_gameover)
		return;

	switch (m_direction)
	{
	case UP:
		break;

	case DOWN:
		break;

	case LEFT:
		break;

	case RIGHT:
		break;
	}

	if (!CheckValidMove())
		m_gameover = true;

}

void Snake::Turn(Direction direction)
{
	if (m_direction == UP && direction == DOWN)
		return;
	if (m_direction == DOWN && direction == UP)
		return;
	if (m_direction == LEFT && direction == RIGHT)
		return;
	if (m_direction == RIGHT && direction == LEFT)
		return;

	m_direction = direction;
}
