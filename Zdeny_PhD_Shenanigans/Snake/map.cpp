#include "stdafx.h"
#include "map.h"

Map::Map(unsigned width, unsigned height): width(width), height(height)
{
};

void Map::Clear()
{
	m_map = Mat::zeros(height, width, CV_32FC3);
}

Mat Map::DrawBody(Snake Bob)
{
	Clear();
	auto& body = Bob.GetBody();

	for (int i = 0; i < body.size(); i++)
	{
		if (i == (body.size() - 1))//head
			m_map.at<Vec3f>(m_map.rows - 1 - body[i].y, body[i].x) = Vec3f(0, 0, 1);
		else//body
			m_map.at<Vec3f>(m_map.rows - 1 - body[i].y, body[i].x) = Vec3f(1, 1, 1);
	}

	return m_map;
}