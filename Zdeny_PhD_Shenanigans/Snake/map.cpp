#include "stdafx.h"
#include "map.h"

Map::Map(unsigned width, unsigned height): width(width), height(height)
{
	//SetFood();
	m_food = Snake::Coordinate(width / 2, height / 2 + 10);
}

void Map::Clear()
{
	m_map = Mat::zeros(height, width, CV_32FC3);
}

Mat Map::Draw(Snake Bob)
{
	Clear();
	auto& body = Bob.GetBody();

	for (int i = 0; i < body.size(); i++)
	{
		if (i == (body.size() - 1))//snake head
			m_map.at<Vec3f>(m_map.rows - 1 - body[i].y, body[i].x) = Vec3f(0, 1, 0);
		else//snake body
			m_map.at<Vec3f>(m_map.rows - 1 - body[i].y, body[i].x) = Vec3f(rand01(), rand01(), rand01());
	}

	m_map.at<Vec3f>(m_map.rows - 1 - m_food.y, m_food.x) = Vec3f(0, 0, 1);

	return m_map;
}

Snake::Coordinate Map::GetFood()
{
	return m_food;
}

void Map::SetFood()
{
	m_food = Snake::Coordinate(rand() % width, rand() % height);
}