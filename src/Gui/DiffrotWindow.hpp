#pragma once
#include "Window.hpp"
#include "Astrophysics/DifferentialRotation.hpp"

class DiffrotWindow : public Window
{
  struct DiffrotParameters
  {
    i32 xsize = 2500;
    i32 ysize = 101;
    i32 idstep = 1;
    i32 idstride = 25;
    f32 thetamax = 50;
    i32 cadence = 45;
    i32 idstart = 18933122;
    i32 xsizeopt = 1;
    i32 ysizeopt = 101;
    i32 popsize = 6;
    std::string dataPath = "/media/zdenyhraz/Zdeny_exSSD/diffrot_month_5000";
    std::string loadPath = "/media/zdenyhraz/Zdeny_exSSD/diffrot_month_5000/xd.json";
  };

  DiffrotParameters mParameters;
  DifferentialRotation::DifferentialRotationData mDiffrotData{
      mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride, mParameters.thetamax, mParameters.cadence, mParameters.idstart};
  f32 mProgress = 0;

public:
  void Render() override;
};
