#pragma once
#include "Window.hpp"
#include "Astrophysics/DifferentialRotation.hpp"
#include "Astrophysics/SolarWindSpeed.hpp"

class AstroWindow : public Window
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

  DiffrotParameters mDiffrotParameters;
  DifferentialRotation::DifferentialRotationData mDiffrotData{mDiffrotParameters.xsize, mDiffrotParameters.ysize, mDiffrotParameters.idstep, mDiffrotParameters.idstride,
      mDiffrotParameters.thetamax, mDiffrotParameters.cadence, mDiffrotParameters.idstart};
  f32 mProgress = 0;
  SolarWindSpeedParameters mSwindParameters;

public:
  void Render() override;
};
