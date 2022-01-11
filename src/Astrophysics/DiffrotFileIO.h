#pragma once

#include "DiffrotResults.h"

void SaveDiffrotResultsToFile(const std::string& dir, const std::string& filename, DiffrotResults* dr, IterativePhaseCorrelation* ipc)
{
  LOG_FUNCTION("SaveDiffrotResultsToFile");

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

  cv::FileStorage fs(path, cv::FileStorage::WRITE);

  std::vector<std::vector<f64>> SourceThetas;
  std::vector<std::vector<f64>> SourceShiftsX;
  std::vector<std::vector<f64>> SourceShiftsY;
  std::vector<std::vector<f64>> SourceOmegasX;
  std::vector<std::vector<f64>> SourceOmegasY;
  dr->GetVecs2D(SourceThetas, SourceShiftsX, SourceShiftsY, SourceOmegasX, SourceOmegasY);
  fs << "SourceThetas" << SourceThetas;
  fs << "SourceShiftsX" << SourceShiftsX;
  fs << "SourceShiftsY" << SourceShiftsY;
  fs << "SourceOmegasX" << SourceOmegasX;
  fs << "SourceOmegasY" << SourceOmegasY;

  i32 SourcePics;
  i32 SourceStride;
  dr->GetParams(SourcePics, SourceStride);
  fs << "SourcePics" << SourcePics;
  fs << "SourceStride" << SourceStride;

  fs << "BandpassL" << ipc->GetBandpassL();
  fs << "BandpassH" << ipc->GetBandpassH();
  fs << "L2size" << ipc->GetL2size();
  // fs << "ApplyWindow" << ipc->GetApplyWindow();
  // fs << "ApplyBandpass" << ipc->GetApplyBandpass();
  fs << "WindowSize" << ipc->GetSize();
}

void LoadDiffrotResultsFromFile(const std::string& path, DiffrotResults* dr)
{
  LOG_FUNCTION("LoadDiffrotResultsFromFile");

  if (!std::filesystem::exists(path))
  {
    LOG_ERROR("File {} does not exist, cannot load", path);
    return;
  }

  cv::FileStorage fs(path, cv::FileStorage::READ);

  std::vector<std::vector<f64>> SourceThetas;
  std::vector<std::vector<f64>> SourceShiftsX;
  std::vector<std::vector<f64>> SourceShiftsY;
  std::vector<std::vector<f64>> SourceOmegasX;
  std::vector<std::vector<f64>> SourceOmegasY;
  fs["SourceThetas"] >> SourceThetas;
  fs["SourceShiftsX"] >> SourceShiftsX;
  fs["SourceShiftsY"] >> SourceShiftsY;
  fs["SourceOmegasX"] >> SourceOmegasX;
  fs["SourceOmegasY"] >> SourceOmegasY;
  dr->SetVecs2DRaw(SourceThetas, SourceShiftsX, SourceShiftsY, SourceOmegasX, SourceOmegasY);

  i32 SourcePics;
  i32 SourceStride;
  fs["SourcePics"] >> SourcePics;
  fs["SourceStride"] >> SourceStride;
  dr->SetParamsRaw(SourcePics, SourceStride);

  f64 L, H;
  i32 winsize, L2size;
  // bool applyWindow, applyBandpass;
  fs["BandpassL"] >> L;
  fs["BandpassH"] >> H;
  fs["L2size"] >> L2size;
  // fs["ApplyWindow"] >> applyWindow;
  // fs["ApplyBandpass"] >> applyBandpass;
  fs["WindowSize"] >> winsize;

  LOG_DEBUG("SourcePics = {}", SourcePics);
  LOG_DEBUG("SourceStride = {}", SourceStride);

  if (!winsize)
    LOG_DEBUG("IPC parameters not specified");

  if (winsize)
    LOG_DEBUG("IPC parameters = {}", std::vector<f64>{L, H, (f64)L2size, 0, 0, (f64)winsize});

  dr->calculated = true;
}
