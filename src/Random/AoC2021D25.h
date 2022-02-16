#include <string>

namespace AoC2021D25
{

struct Seafloor
{
  explicit Seafloor(const std::string& str) { Parse(str); };

  enum Entity : u8
  {
    None,
    Right,
    Down,
  };

  std::vector<std::vector<Entity>> currentmap;
  std::vector<std::vector<Entity>> nextmap;
  cv::Mat pic;
  i32 width = 0;
  i32 height = 0;
  i32 steps = 0;

  void Parse(const std::string& str)
  {
    i32 x = 0;
    i32 y = 0;
    std::vector<std::tuple<cv::Point, Entity>> cucumbers;
    for (const auto character : str)
    {
      if (character == '\n')
      {
        y++;
        width = x;
        x = 0;
        continue;
      }

      if (character == '>')
        cucumbers.emplace_back(cv::Point{x, y}, Right);

      if (character == 'v')
        cucumbers.emplace_back(cv::Point{x, y}, Down);

      x++;
    }
    height = y + 1; // string does not end with newline

    currentmap.resize(height);
    for (auto& row : currentmap)
      row.resize(width);

    for (i32 r = 0; r < height; r++)
      for (i32 c = 0; c < width; c++)
        currentmap[r][c] = None;
    nextmap = currentmap;

    for (const auto& [position, entity] : cucumbers)
      currentmap[position.y][position.x] = entity;

    Debug();
  }

  cv::Point GetShift(Entity entity)
  {
    switch (entity)
    {
    case Right:
      return {1, 0};
    case Down:
      return {0, 1};
    default:
      return {0, 0};
    }
  }

  std::string GetString(Entity entity)
  {
    switch (entity)
    {
    case Right:
      return ">";
    case Down:
      return "v";
    default:
      return ".";
    }
  }

  bool EntityStep(Entity ent)
  {
    for (i32 r = 0; r < height; r++)
    {
      for (i32 c = 0; c < width; c++)
      {
        if (currentmap[r][c] == None)
          continue;

        if (currentmap[r][c] != ent)
        {
          nextmap[r][c] = currentmap[r][c]; // not your turn - stay
          continue;
        }

        const auto shift = GetShift(ent);
        const i32 nr = (r + shift.y) % height;
        const i32 nc = (c + shift.x) % width;

        if (currentmap[nr][nc] == None)
          nextmap[nr][nc] = ent; // move
        else
          nextmap[r][c] = ent; // blocked
      }
    }
    return SwapMaps();
  }

  bool SwapMaps()
  {
    bool moved = currentmap != nextmap;
    currentmap = nextmap;
    for (i32 r = 0; r < height; r++)
      for (i32 c = 0; c < width; c++)
        nextmap[r][c] = None;
    return moved;
  }

  bool Step()
  {
    bool movedRight = EntityStep(Right);
    bool movedDown = EntityStep(Down);
    steps++;
    Debug();
    return (movedRight or movedDown) and steps < 1e5;
  }

  i32 GetStabilitySteps()
  {
    while (Step())
    {
    }
    return steps;
  }

  void Debug()
  {
    pic = cv::Mat::zeros(height, width, CV_32F);
    for (i32 r = 0; r < height; r++)
      for (i32 c = 0; c < width; c++)
        pic.at<f32>(r, c) = currentmap[r][c];

    if (width < 50 or height < 50)
      cv::resize(pic, pic, cv::Size(width * 10, height * 10), cv::INTER_NEAREST);

    Plot2D::Set("cucumbers");
    Plot2D::Plot(pic);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    if (width > 10 or height > 10)
      return; // too long for text output

    LOG_DEBUG("Step {}:", steps);
    for (i32 r = 0; r < height; r++)
    {
      std::string line;
      for (i32 c = 0; c < width; c++)
        line += GetString(currentmap[r][c]);
      LOG_DEBUG("{}", line);
    }
  }
};

u64 AoC2021D25()
{
  std::string input = R"(..........
.>v....v..
.......>..
..........)";
  return Seafloor(input).GetStabilitySteps();
}
}