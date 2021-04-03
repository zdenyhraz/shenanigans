#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

using namespace cv;

struct Fractalset
{
  double zoom;
  int sliderZoom;
  int sliderLog;
  int sliderMaxiter;
  int sliderMagntresh;
  int fractalWidth;
  int fractalHeight;
  Point2d fractalCenter;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double magnTresh;
  int maxiter;

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
    zoom *= pow(2., ((double)sliderZoom - 50) / 50 * 3.);
    magnTresh = ((double)sliderMagntresh - 50) / 50 + 2.;
    maxiter = sliderMaxiter;
    // std::cout << "Fractal center: " << fractalCenter.x << " + " << fractalCenter.y << "i, delta_x: " << xmax - xmin << ", eps: " << (xmax - xmin) / fractalWidth << ", zoom: " << zoom << "(" << 1. /
    // zoom << " x)" << std::endl;
  }
};

Mat computeFractal(Fractalset& fractalset);

void mouseEventFractal(int event, int x, int y, int flags, void* ptr);
