#pragma once

class Map;

class Snake
{
public:
  Snake(Map& map);

  struct Coordinate
  {
    Coordinate() {}
    Coordinate(i32 x_, i32 y_) : x(x_), y(y_) {}

    i32 x = 0;
    i32 y = 0;

    bool operator==(const Coordinate& other) { return (x == other.x) and (y == other.y); }
    bool operator!=(const Coordinate& other) { return !(*this == other); }
  };

  enum Direction
  {
    UP,
    DOWN,
    LEFT,
    RIGHT
  };

  void Tick();

  void Turn(Direction direction);

  std::vector<Coordinate>& GetBody();

  bool GetGameOver();

private:
  std::vector<Coordinate> m_body;
  Direction m_direction;
  bool m_gameover;
  Map& m_map;

  bool CheckValidMove();
};