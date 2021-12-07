#pragma once

#include "map.h"

static void SnakeGame()
{
  bool stop = false;
  int size = 50;
  Map map(size, size);
  Snake snake(map);
  int c;

  while (!stop)
  {
    c = cv::waitKey(50);

    switch (c)
    {
    case 27: // escape
      stop = true;
      break;

    case 100: // right arrow
      snake.Turn(Snake::RIGHT);
      break;

    case 97: // left arrow
      snake.Turn(Snake::LEFT);
      break;

    case 119: // up arrow
      snake.Turn(Snake::UP);
      break;

    case 115: // down arrow
      snake.Turn(Snake::DOWN);
      break;
    }

    if (stop)
    {
      QMessageBox msgBox;
      msgBox.setText("Thanks for playing xd");
      msgBox.exec();
      break;
    }

    snake.Tick();

    if (snake.GetGameOver())
    {
      QMessageBox msgBox;
      msgBox.setText("Bob the snake is dead :((    	\n\n      </3                 R.I.P. Bob");
      msgBox.exec();
      break;
    }

    showimg(map.Draw(snake), "snake");
  }

  cv::destroyWindow("snake");
}
