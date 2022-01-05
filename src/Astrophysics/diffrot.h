#pragma once
#include "IPC/IterativePhaseCorrelation.h"
#include "diffrotResults.h"

static constexpr i32 plusminusbufer = 4; // even!
static constexpr i32 yshow = 1400;       // ipc show y index

struct DiffrotSettings
{
  i32 pics;
  i32 ys;
  i32 sPic;
  i32 dPic;
  i32 vFov;
  i32 dSec;
  bool medianFilter;
  bool movavgFilter;
  i32 medianFilterSize;
  i32 movavgFilterSize;
  bool visual;
  i32 sy;
  bool pred;
  bool speak = true;
  std::string savepath;
  bool video = false;
};

DiffrotResults calculateDiffrotProfile(const IterativePhaseCorrelation& ipc, FitsTime& time, const DiffrotSettings& drset);

void loadFitsFuzzy(FitsImage& pic, FitsTime& time, i32& lag);

void calculateOmegas(const FitsImage& pic1, const FitsImage& pic2, std::vector<f64>& shiftsX, std::vector<f64>& shiftsY, std::vector<f64>& thetas, std::vector<f64>& omegasX, std::vector<f64>& omegasY,
    const IterativePhaseCorrelation& ipc, const DiffrotSettings& drset, f64 R, f64 theta0, f64 dy, i32 lag1, i32 lag2, i32 predShift);
