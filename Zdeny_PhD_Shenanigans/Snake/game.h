#pragma once
#include "stdafx.h"
#include "map.h"

using namespace cv;

void SnakeGame()
{
	while (1)
	{
		srand(time(0));
		int size = 70;
		Map map(size, size);
		Snake snake(map);

		while (1)
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(50));
			int c = cvWaitKey(100);
			cout << "c=" << c << endl;

			switch (c)
			{
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

			snake.Tick();

			if (snake.GetGameOver())
				break;

			showimg(map.Draw(snake), "snake");
		}
		QMessageBox msgBox;
		msgBox.setText("Bob the snake is dead :(((((\n\n        R.I.P.");
		msgBox.exec();
	}
}