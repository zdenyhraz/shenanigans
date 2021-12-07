
#include "Draw/showsave.h"
#include "fractal.h"

void mouseEventFractal(int event, int x, int y, int flags, void* ptr)
{
  if (event == cv::EVENT_LBUTTONDOWN)
  {
    Fractalset* set = (Fractalset*)ptr;
    set->fractalCenter.x = ((double)x / set->fractalWidth) * (set->xmax - set->xmin) + set->xmin;
    set->fractalCenter.y = -((double)y / set->fractalHeight) * (set->ymax - set->ymin) + set->ymax;
    set->computeDependent();
    computeFractal(*set);
  }
}

cv::Mat computeFractal(Fractalset& fractalset)
{
  using namespace std::complex_literals;

  cv::Mat Fractal = cv::Mat::zeros(fractalset.fractalHeight, fractalset.fractalWidth, CV_32F);
  std::complex<double> startZ = 0. + 0.i;
  std::cout << "Computing Fractal...";
#pragma omp parallel for
  for (int r = 0; r < fractalset.fractalHeight; r++)
  {
    for (int c = 0; c < fractalset.fractalWidth; c++)
    {
      std::complex<double> C = (double)c / (fractalset.fractalWidth) * (fractalset.xmax - fractalset.xmin) + fractalset.xmin +
                               1.i * (-(double)r / (fractalset.fractalHeight) * (fractalset.ymax - fractalset.ymin) + fractalset.ymax);
      std::complex<double> Z = startZ;
      double M = 0.;
      int finaliter = 0;
      for (int iter = 0; iter < fractalset.maxiter; iter++)
      {
        finaliter++;
        Z = pow(Z, 2) + C; // mandel
        // Z = pow(sin(Z), 2)+C;//flower
        // Z = 1. / (pow(Z, 4) + 1.) + C;//interesting-M
        // Z = pow(C, Z);//asscrack
        M = sqrt(pow(Z.real(), 2) + pow(Z.imag(), 2));
        if (M > fractalset.magnTresh)
          break;
      }
      Fractal.at<float>(r, c) = (double)finaliter;
    }
  }

  for (int logiter = 0; logiter < fractalset.sliderLog; logiter++)
  {
    Fractal += cv::Scalar::all(1.);
    log(Fractal, Fractal);
  }

  cv::namedWindow("Fractal", cv::WINDOW_NORMAL);
  cv::setMouseCallback("Fractal", mouseEventFractal, &fractalset);

  cv::createTrackbar("zoom", "Fractal", &fractalset.sliderZoom, 100);
  cv::createTrackbar("loglim", "Fractal", &fractalset.sliderLog, 100);
  cv::createTrackbar("maxiter", "Fractal", &fractalset.sliderMaxiter, 100);
  cv::createTrackbar("magnTresh", "Fractal", &fractalset.sliderMagntresh, 100);

  cv::setTrackbarMax("zoom", "Fractal", 100);
  cv::setTrackbarMax("loglim", "Fractal", 50);
  cv::setTrackbarMax("maxiter", "Fractal", 1e4);
  cv::setTrackbarMax("magnTresh", "Fractal", 100);

  showimg(Fractal, "Fractal");
  showimg(Fractal, "Fractal_JET", true);

  std::cout << "Fractal created." << std::endl;
  return Fractal;
}
