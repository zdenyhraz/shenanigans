#include "map.h"

Map::Map(unsigned width, unsigned height): width(width), height(height)
{
	map = Mat::zeros(height, width, CV_32F);
};

void Map::DrawBody(Snake Bob)
{
}