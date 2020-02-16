#pragma once
#include "stdafx.h"
#include "map.h"

using namespace cv;
static const char* windowname = "snake";

void SnakeGame()
{
	bool stop = false;

	while (1)
	{
		srand(time(0));
		int size = 50;
		Map map(size, size);
		Snake snake(map);

		while (1)
		{
			int c = cvWaitKey(100);
			cout << "c=" << c << endl;

			switch (c)
			{
			case 27: //escape
				stop = true;
				break;

			case 2555904: // right arrow
				snake.Turn(Snake::RIGHT);
				break;

			case 2424832: // left arrow
				snake.Turn(Snake::LEFT);
				break;

			case 2490368: // up arrow
				snake.Turn(Snake::UP);
				break;

			case 2621440: // down arrow
				snake.Turn(Snake::DOWN);
				break;
			}

			if (stop)
				break;

			snake.Tick();

			if (snake.GetGameOver())
				break;

			showimg(map.Draw(snake), windowname);
		}

		if (stop)
		{
			cvDestroyWindow(windowname);
			break;
		}

		QMessageBox msgBox;
		msgBox.setText("Bob the snake is dead :((    	\n\n      </3                 R.I.P. Bob");
		msgBox.exec();
	}
}