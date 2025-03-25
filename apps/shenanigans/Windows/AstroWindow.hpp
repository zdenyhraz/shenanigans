#pragma once
#include "Window.hpp"
#include "Astrophysics/DifferentialRotation.hpp"
#include "Astrophysics/SolarWindSpeed.hpp"

class AstroWindow : public Window
{
  struct DiffrotParameters
  {
    int xsize = 2500;
    int ysize = 101;
    int idstep = 1;
    int idstride = 25;
    float thetamax = 50;
    int cadence = 45;
    int idstart = 18933122;
    int xsizeopt = 1;
    int ysizeopt = 101;
    int popsize = 6;
    std::string dataPath = "/media/zdenyhraz/Zdeny_exSSD/diffrot_month_5000";
    std::string loadPath = "/media/zdenyhraz/Zdeny_exSSD/diffrot_month_5000/xd.json";
  };

  DiffrotParameters mDiffrotParameters;
  DifferentialRotation::DifferentialRotationData mDiffrotData{mDiffrotParameters.xsize, mDiffrotParameters.ysize, mDiffrotParameters.idstep, mDiffrotParameters.idstride,
      mDiffrotParameters.thetamax, mDiffrotParameters.cadence, mDiffrotParameters.idstart};
  float mProgress = 0;
  SolarWindSpeedParameters mSwindParameters;

public:
  void Render() override;
};
