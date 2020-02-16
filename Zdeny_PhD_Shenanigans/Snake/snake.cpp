#pragma once
#include "stdafx.h"
#include "snake.h"
#include "map.h"

using namespace cv;

Snake::Snake(Map& map) : m_map(map)
{
	//game just started
	m_gameover = false;
	
	//3 units long snake
	m_body.push_back(Coordinate(map.width / 2, map.height / 2 - 1));
	m_body.push_back(Coordinate(map.width / 2, map.height / 2));
	m_body.push_back(Coordinate(map.width / 2, map.height / 2 + 1));

	//heading up
	m_direction = UP;
}

bool Snake::CheckValidMove()
{
	auto& head = m_body.back();
	bool valid = true;

	//check boundaries
	valid = valid && head.x < m_map.width;
	valid = valid && head.x >= 0;
	valid = valid && head.y < m_map.height;
	valid = valid && head.y >= 0;

	//check self eat
	if (std::find(m_body.begin(), m_body.end() - 1, head) != (m_body.end() - 1))
		valid = false;

	return valid;
}

void Snake::Tick()
{
	if (m_gameover)
		return;

	auto& head = m_body.back();

	switch (m_direction)
	{
	case UP:
		m_body.push_back(Coordinate(head.x, head.y + 1));
		break;

	case DOWN:
		m_body.push_back(Coordinate(head.x, head.y - 1));
		break;

	case LEFT:
		m_body.push_back(Coordinate(head.x - 1, head.y));
		break;

	case RIGHT:
		m_body.push_back(Coordinate(head.x + 1, head.y));
		break;
	}

	//if not food then poop front
	if (m_body.back() != m_map.GetFood())
		m_body.erase(m_body.begin());
	else
		m_map.SetFood();

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

std::vector<Snake::Coordinate> Snake::GetBody()
{
	return m_body;
}

bool Snake::GetGameOver()
{
	return m_gameover;
}

