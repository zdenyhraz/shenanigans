#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "IPC/IterativePhaseCorrelation.h"
#include "diffrotResults.h"

static constexpr int plusminusbufer = 4; // even!
static constexpr int yshow = 1400;       // ipc show y index

struct DiffrotSettings
{
  int pics;
  int ys;
  int sPic;
  int dPic;
  int vFov;
  int dSec;
  bool medianFilter;
  bool movavgFilter;
  int medianFilterSize;
  int movavgFilterSize;
  bool visual;
  int sy;
  bool pred;
  bool speak = true;
  std::string savepath;
  bool video = false;
};

DiffrotResults calculateDiffrotProfile(const IterativePhaseCorrelation &ipc, FitsTime &time, const DiffrotSettings &drset);

void loadFitsFuzzy(FitsImage &pic, FitsTime &time, int &lag);

void calculateOmegas(const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &shiftsY, std::vector<double> &thetas, std::vector<double> &omegasX, std::vector<double> &omegasY, std::vector<double> &image, std::vector<std::vector<double>> &predicXs, const IterativePhaseCorrelation &ipc, const DiffrotSettings &drset, double R, double theta0, double dy, int lag1, int lag2, int predShift);
