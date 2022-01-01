#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

struct Fractalset
{
  f64 zoom;
  i32 sliderZoom;
  i32 sliderLog;
  i32 sliderMaxiter;
  i32 sliderMagntresh;
  i32 fractalWidth;
  i32 fractalHeight;
  cv::Point2d fractalCenter;
  f64 xmin;
  f64 xmax;
  f64 ymin;
  f64 ymax;
  f64 magnTresh;
  i32 maxiter;

  Fractalset() // constructor
  {
    // std::cout << "Fractal object created" << std::endl;
    zoom = 1.;
    sliderZoom = 50;
    sliderLog = 1;
    sliderMaxiter = 1000;
    sliderMagntresh = 50;
    fractalWidth = 1000;
    fractalCenter.x = 0.;
    fractalCenter.y = 0.;
    magnTresh = 2.;
    computeDependent();
  }

  void computeDependent()
  {
    xmin = fractalCenter.x - 1.5 * zoom;
    xmax = fractalCenter.x + 1.5 * zoom;
    ymin = fractalCenter.y - zoom;
    ymax = fractalCenter.y + zoom;
    fractalHeight = (ymax - ymin) / (xmax - xmin) * fractalWidth;
    zoom *= pow(2., ((f64)sliderZoom - 50) / 50 * 3.);
    magnTresh = ((f64)sliderMagntresh - 50) / 50 + 2.;
    maxiter = sliderMaxiter;
    // std::cout << "Fractal center: " << fractalCenter.x << " + " << fractalCenter.y << "i, delta_x: " << xmax - xmin << ", eps: " << (xmax - xmin) / fractalWidth << ", zoom: " << zoom << "(" << 1. /
    // zoom << " x)" << std::endl;
  }
};

cv::Mat computeFractal(Fractalset& fractalset);

void mouseEventFractal(i32 event, i32 x, i32 y, i32 flags, void* ptr);
