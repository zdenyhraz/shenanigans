#pragma once

struct DiffrotParameters
{
  i32 xsize = 2500;
  i32 ysize = 101;
  i32 idstep = 1;
  i32 idstride = 25;
  f32 thetamax = 50;
  i32 cadence = 45;
  std::string dataPath = "../data/diffrot_month_5000";
  i32 idstart = 123132;

  i32 xsizeopt = 1;
  i32 ysizeopt = 101;
  i32 popsize = 6;
};

class DiffrotWindow
{
public:
  static void Initialize();
  static void Render();

private:
  inline static DiffrotParameters mParameters;
};
