#pragma once

struct FormationEntity
{
  cv::Point2d formationOffset;
  cv::Point2d position;
};

std::vector<FormationEntity> CreateFormation()
{
  // TODO: bigger entities
  std::vector<FormationEntity> entities;
  entities.push_back({.formationOffset = cv::Point2d(0, 0)});

  entities.push_back({.formationOffset = cv::Point2d(-50, -50)});
  entities.push_back({.formationOffset = cv::Point2d(50, -50)});

  entities.push_back({.formationOffset = cv::Point2d(-100, -100)});
  entities.push_back({.formationOffset = cv::Point2d(0, -100)});
  entities.push_back({.formationOffset = cv::Point2d(100, -100)});

  entities.push_back({.formationOffset = cv::Point2d(-150, -150)});
  entities.push_back({.formationOffset = cv::Point2d(-50, -150)});
  entities.push_back({.formationOffset = cv::Point2d(50, -150)});
  entities.push_back({.formationOffset = cv::Point2d(150, -150)});
  return entities;
}

struct PathData
{
  std::vector<cv::Point> path;
  std::vector<cv::Point> leftBoundary;
  std::vector<cv::Point> rightBoundary;
  std::vector<cv::Point2d> tangents;
  std::vector<cv::Point2d> normals;
  std::vector<cv::Rect> gridLocations;
};

cv::Rect GetEntityGridLocation(const cv::Point2d& position, double gridSize)
{
  const int gridX = static_cast<int>(position.x / gridSize) * gridSize;
  const int gridY = static_cast<int>(position.y / gridSize) * gridSize;
  return cv::Rect(gridX, gridY, gridSize, gridSize);
}

PathData CreatePath(int mapSize, double gridSize)
{
  PathData pathData;
  for (double t = 0; t <= 1; t += 0.001)
  {
    cv::Point pathPoint;
    pathPoint.x = mapSize * (0.5 + 0.25 * std::cos(6.28 * t));
    pathPoint.y = mapSize * (0.5 + 0.25 * std::sin(6.28 * t));
    pathData.path.push_back(pathPoint);

    cv::Point2d pathTangent;
    pathTangent.x = -std::sin(6.28 * t);
    pathTangent.y = std::cos(6.28 * t);
    pathData.tangents.push_back(pathTangent);

    cv::Point2d pathNormal;
    pathNormal.x = -std::cos(6.28 * t);
    pathNormal.y = -std::sin(6.28 * t);
    pathData.normals.push_back(pathNormal);

    double width = std::clamp((1 + std::sin(6.28 * t)) * 100, 50., 200.);
    pathData.leftBoundary.push_back(cv::Point2d(pathPoint) + pathNormal * width);
    pathData.rightBoundary.push_back(cv::Point2d(pathPoint) - pathNormal * width);

    // always add at least one square
    const auto gridLocation = GetEntityGridLocation(cv::Point2d(pathPoint), gridSize);
    if (std::ranges::find(pathData.gridLocations, gridLocation) == pathData.gridLocations.end())
      pathData.gridLocations.push_back(gridLocation);
    // add along normal
    for (double normalOffset = -width + gridSize; normalOffset <= width - gridSize; normalOffset += gridSize)
    {
      const auto gridLocation = GetEntityGridLocation(cv::Point2d(pathPoint) + pathNormal * normalOffset, gridSize);
      if (std::ranges::find(pathData.gridLocations, gridLocation) == pathData.gridLocations.end())
        pathData.gridLocations.push_back(gridLocation);
    }
  }
  return pathData;
}

void FormationPathfinding()
{
  LOG_FUNCTION;
  const int mapSize = 1024;
  cv::Scalar pathColor(0, 0, 255);
  cv::Scalar boundaryColor(255, 255, 255);
  cv::Scalar tangentColor(255, 255, 0);
  cv::Scalar normalColor(0, 255, 255);
  cv::Scalar entityColor(0, 255, 0);
  cv::Scalar gridColor(0, 255, 0);
  const int entitySize = 10;
  const double tstep = 0.0001;
  const double gridSize = 50;

  auto path = CreatePath(mapSize, gridSize);
  auto entities = CreateFormation();
  double t = 0;
  cv::Mat image;
  while (true)
  {
    image = cv::Mat::zeros(mapSize, mapSize, CV_8UC3);
    cv::polylines(image, path.path, true, pathColor, 2, cv::LINE_AA);
    cv::polylines(image, path.leftBoundary, true, boundaryColor, 1, cv::LINE_AA);
    cv::polylines(image, path.rightBoundary, true, boundaryColor, 1, cv::LINE_AA);
    for (auto& gridLocation : path.gridLocations)
      cv::rectangle(image, gridLocation, cv::Scalar(255, 0, 0), 1, cv::LINE_AA);

    // update & draw path
    const int idx = static_cast<int>(t * path.path.size()) % path.path.size();
    cv::Point2d pathPoint = path.path[idx];
    cv::Point2d pathTangent = path.tangents[idx];
    cv::Point2d pathNormal = path.normals[idx];
    cv::line(image, pathPoint, pathPoint + pathNormal * 50, normalColor, 2, cv::LINE_AA);
    cv::line(image, pathPoint, pathPoint + pathTangent * 50, tangentColor, 2, cv::LINE_AA);

    // update & draw entities
    std::vector<cv::Rect> occupiedGridLocations;
    for (auto& entity : entities)
    {
      // desired position
      entity.position = pathPoint + pathNormal * entity.formationOffset.x + pathTangent * entity.formationOffset.y;
      // resolve collisions
      auto gridLocation = GetEntityGridLocation(entity.position, gridSize);
      if (std::ranges::find(occupiedGridLocations, gridLocation) == occupiedGridLocations.end() and std::ranges::find(path.gridLocations, gridLocation) != path.gridLocations.end())
      {
        // grid location not occupied and reachable
        occupiedGridLocations.push_back(gridLocation);
      }
      else
      {
        // grid location occupied or not reachable
      }

      cv::circle(image, entity.position, entitySize, entityColor, -1);
      cv::circle(image, entity.position, 3, cv::Scalar(0, 0, 255), -1);
    }

    for (auto& gridLocation : occupiedGridLocations)
      cv::rectangle(image, gridLocation, cv::Scalar(0, 255, 255), 1, cv::LINE_AA);

    t += tstep;
    Plot::Plot({.name = "formation pathfinding", .z = image, .cmap = "gray"});
  }
}
