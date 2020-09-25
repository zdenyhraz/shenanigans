#pragma once
#include "stdafx.h"
#include "diffrotResults.h"

void SaveDiffrotResultsToFile(const std::string &dir, const std::string &filename, DiffrotResults *dr, IterativePhaseCorrelation *ipc)
{
  LOG_STARTEND("Saving diffrot results ...", "Diffrot results saved");

  std::string path = dir + filename + ".diffrot";

  if (std::filesystem::exists(path))
  {
    LOG_ERROR("File {} already exists, cannot save", path);
    return;
  }

  if (!dr->calculated)
  {
    LOG_ERROR("Diffrot results not yet calculated!");
    return;
  }

  FileStorage fs(path, FileStorage::WRITE);

  std::vector<std::vector<double>> SourceThetas;
  std::vector<std::vector<double>> SourceShiftsX;
  std::vector<std::vector<double>> SourceShiftsY;
  std::vector<std::vector<double>> SourceOmegasX;
  std::vector<std::vector<double>> SourceOmegasY;
  dr->GetVecs2D(SourceThetas, SourceShiftsX, SourceShiftsY, SourceOmegasX, SourceOmegasY);
  fs << "SourceThetas" << SourceThetas;
  fs << "SourceShiftsX" << SourceShiftsX;
  fs << "SourceShiftsY" << SourceShiftsY;
  fs << "SourceOmegasX" << SourceOmegasX;
  fs << "SourceOmegasY" << SourceOmegasY;

  int SourcePics;
  int SourceStride;
  dr->GetParams(SourcePics, SourceStride);
  fs << "SourcePics" << SourcePics;
  fs << "SourceStride" << SourceStride;

  fs << "BandpassL" << ipc->GetBandpassL();
  fs << "BandpassH" << ipc->GetBandpassH();
  fs << "L2size" << ipc->GetL2size();
  fs << "ApplyWindow" << ipc->GetApplyWindow();
  fs << "ApplyBandpass" << ipc->GetApplyBandpass();
  fs << "WindowSize" << ipc->GetSize();
}

void LoadDiffrotResultsFromFile(const std::string &path, DiffrotResults *dr)
{
  LOG_STARTEND("Loading diffrot results ...", "Diffrot results loaded");

  if (!std::filesystem::exists(path))
  {
    LOG_ERROR("File {} does not exist, cannot load", path);
    return;
  }

  FileStorage fs(path, FileStorage::READ);

  std::vector<std::vector<double>> SourceThetas;
  std::vector<std::vector<double>> SourceShiftsX;
  std::vector<std::vector<double>> SourceShiftsY;
  std::vector<std::vector<double>> SourceOmegasX;
  std::vector<std::vector<double>> SourceOmegasY;
  fs["SourceThetas"] >> SourceThetas;
  fs["SourceShiftsX"] >> SourceShiftsX;
  fs["SourceShiftsY"] >> SourceShiftsY;
  fs["SourceOmegasX"] >> SourceOmegasX;
  fs["SourceOmegasY"] >> SourceOmegasY;
  dr->SetVecs2DRaw(SourceThetas, SourceShiftsX, SourceShiftsY, SourceOmegasX, SourceOmegasY);

  int SourcePics;
  int SourceStride;
  fs["SourcePics"] >> SourcePics;
  fs["SourceStride"] >> SourceStride;
  dr->SetParamsRaw(SourcePics, SourceStride);

  double L, H;
  int winsize, L2size;
  bool applyWindow, applyBandpass;
  fs["BandpassL"] >> L;
  fs["BandpassH"] >> H;
  fs["L2size"] >> L2size;
  fs["ApplyWindow"] >> applyWindow;
  fs["ApplyBandpass"] >> applyBandpass;
  fs["WindowSize"] >> winsize;

  LOG_DEBUG("SourcePics = {}", SourcePics);
  LOG_DEBUG("SourceStride = {}", SourceStride);
  LOG_DEBUG_IF(!winsize, "IPC parameters not specified");
  LOG_DEBUG_IF(winsize, "IPC parameters = {}", std::vector<double>{L, H, (double)L2size, (double)applyWindow, (double)applyBandpass, (double)winsize});

  dr->calculated = true;
}
