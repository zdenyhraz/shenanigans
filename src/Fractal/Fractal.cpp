
#include "Fractal.h"

void mouseEventFractal(i32 event, i32 x, i32 y, i32 flags, void* ptr)
{
  if (event == cv::EVENT_LBUTTONDOWN)
  {
    Fractalset* set = reinterpret_cast<Fractalset*>(ptr);
    set->fractalCenter.x = ((f64)x / set->fractalWidth) * (set->xmax - set->xmin) + set->xmin;
    set->fractalCenter.y = -((f64)y / set->fractalHeight) * (set->ymax - set->ymin) + set->ymax;
    set->computeDependent();
    computeFractal(*set);
  }
}

cv::Mat computeFractal(Fractalset& fractalset)
{
  using namespace std::complex_literals;

  cv::Mat Fractal = cv::Mat::zeros(fractalset.fractalHeight, fractalset.fractalWidth, CV_32F);
  std::complex<f64> startZ = 0. + 0.i;
  std::cout << "Computing Fractal...";
#pragma omp parallel for
  for (i32 r = 0; r < fractalset.fractalHeight; r++)
  {
    for (i32 c = 0; c < fractalset.fractalWidth; c++)
    {
      std::complex<f64> C = (f64)c / (fractalset.fractalWidth) * (fractalset.xmax - fractalset.xmin) + fractalset.xmin +
                            1.i * (-(f64)r / (fractalset.fractalHeight) * (fractalset.ymax - fractalset.ymin) + fractalset.ymax);
      std::complex<f64> Z = startZ;
      f64 M = 0.;
      i32 finaliter = 0;
      for (i32 iter = 0; iter < fractalset.maxiter; iter++)
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
      Fractal.at<f32>(r, c) = (f64)finaliter;
    }
  }

  for (i32 logiter = 0; logiter < fractalset.sliderLog; logiter++)
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
