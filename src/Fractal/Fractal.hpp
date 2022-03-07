#pragma once

struct Fractalset
{
  f64 zoom = 1.;
  i32 sliderZoom = 50;
  i32 sliderLog = 1;
  i32 sliderMaxiter = 1000;
  i32 sliderMagntresh = 50;
  i32 fractalWidth = 1000;
  i32 fractalHeight;
  cv::Point2d fractalCenter{0., 0.};
  f64 xmin = -1;
  f64 xmax = 1;
  f64 ymin = -1;
  f64 ymax = 1;
  f64 magnTresh = 2.;
  i32 maxiter = 1;

  Fractalset() { computeDependent(); }

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
  }
};

cv::Mat computeFractal(Fractalset& fractalset);

void mouseEventFractal(i32 event, i32 x, i32 y, i32 flags, void* ptr);
