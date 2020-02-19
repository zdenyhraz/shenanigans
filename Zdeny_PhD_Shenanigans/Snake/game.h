#pragma once
#include "stdafx.h"
#include "map.h"

using namespace cv;
static const char* windowname = "snake";

void SnakeGame()
{
	srand(time(0));
	bool stop = false;

	while (1)
	{
		int size = 50;
		Map map(size, size);
		Snake snake(map);
		int c;

		while (1)
		{
			c = cvWaitKey(50);
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

void SnakeGame2()
{
	srand(time(0));
	bool stop = false;
	int size = 50;
	Map map(size, size);
	Snake snake(map);
	Snake::Direction dir = Snake::UP;
	int c;

	auto keyboardthread = [&](bool gameover)
	{
		cout << "<entering keyboard thread>" << endl;
		while (!gameover)
		{
			c = cvWaitKey(50);
			cout << "<keyboard thread> c=" << c << endl;
			switch (c)
			{
			case 27: //escape
				stop = true;
				break;

			case 2555904: // right arrow
				dir = Snake::RIGHT;
				break;

			case 2424832: // left arrow
				dir = Snake::LEFT;
				break;

			case 2490368: // up arrow
				dir = Snake::UP;
				break;

			case 2621440: // down arrow
				dir = Snake::DOWN;
				break;
			}
		}
	};

	auto snakethread = [&](bool gameover)
	{
		cout << "<entering snake thread>" << endl;
		while (!gameover)
		{
			cout << "<snake thread>" << endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			snake.Turn(dir);
			snake.Tick();

			if (snake.GetGameOver())
			{
				return true;
			}

			showimg(map.Draw(snake), windowname);
		}
	};

	volatile bool gameover;

	while (!stop)//repeat games
	{
		gameover = false;

		std::thread tsn(snakethread, std::ref(gameover));
		std::thread tkb(keyboardthread, std::ref(gameover));
		
		tsn.join();
		cout << "snake thread joined" << endl;
		tkb.join();
		cout << "keyboard thread joined" << endl;

		QMessageBox msgBox;
		msgBox.setText("Bob the snake is dead :((    	\n\n      </3                 R.I.P. Bob");
		msgBox.exec();
	}
	cvDestroyWindow(windowname);
}