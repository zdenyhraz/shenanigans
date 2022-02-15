#include <string>

u64 AoC2021D5()
{
  std::stringstream inputStream(R"(629,581,123,75
921,643,452,643
498,588,503,593
861,137,102,896
603,339,603,137
138,738,117,738)");

  cv::Mat mat = cv::Mat::zeros(1000, 1000, CV_16U);
  std::string inputline;
  while (std::getline(inputStream, inputline, '\n'))
  {
    std::stringstream inputlineStream(inputline);
    std::string coord;
    std::vector<std::string> coords;
    while (std::getline(inputlineStream, coord, ','))
      coords.push_back(coord);

    auto x1 = std::stoi(coords[0]);
    auto y1 = std::stoi(coords[1]);
    auto x2 = std::stoi(coords[2]);
    auto y2 = std::stoi(coords[3]);

    LOG_DEBUG("Processing coords [{},{}]->[{},{}]", x1, y1, x2, y2);

    i32 stepx = x1 == x2 ? 0 : x2 - x1 > 0 ? 1 : -1;
    i32 stepy = y1 == y2 ? 0 : y2 - y1 > 0 ? 1 : -1;
    for (i32 step = 0; step <= std::max(std::abs(x2 - x1), std::abs(y2 - y1)); ++step)
      mat.at<u16>(x1 + step * stepx, y1 + step * stepy)++;
  }

  i32 result = 0;
  for (i32 r = 0; r < mat.rows; r++)
    for (i32 c = 0; c < mat.cols; c++)
      if (mat.at<u16>(r, c) >= 2)
        result++;

  return result;
}