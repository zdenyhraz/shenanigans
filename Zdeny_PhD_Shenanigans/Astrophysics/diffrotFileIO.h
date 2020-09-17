#pragma once
#include "stdafx.h"
#include "diffrotResults.h"

void SaveDiffrotResultsToFile(const std::string &dir, const std::string &filename, DiffrotResults *dr, IPCsettings *ipc)
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

  std::vector<double> SourceThetasavg;
  std::vector<double> SourceOmegasXavg;
  std::vector<double> SourceOmegasYavg;
  std::vector<double> SourceShiftsXavg;
  std::vector<double> SourceShiftsYavg;
  dr->GetVecs1D(SourceThetasavg, SourceOmegasXavg, SourceOmegasYavg, SourceShiftsXavg, SourceShiftsYavg);
  fs << "SourceThetasavg" << SourceThetasavg;
  fs << "SourceOmegasXavg" << SourceOmegasXavg;
  fs << "SourceOmegasYavg" << SourceOmegasYavg;
  fs << "SourceShiftsXavg" << SourceShiftsXavg;
  fs << "SourceShiftsYavg" << SourceShiftsYavg;

  std::vector<std::vector<double>> SourceShiftsX;
  std::vector<std::vector<double>> SourceShiftsY;
  dr->GetVecs2D(SourceShiftsX, SourceShiftsY);
  fs << "SourceShiftsX" << SourceShiftsX;
  fs << "SourceShiftsY" << SourceShiftsY;

  Mat SourceImage;
  Mat SourceFlowX;
  Mat SourceFlowY;
  dr->GetMats(SourceImage, SourceFlowX, SourceFlowY);
  fs << "SourceImage" << SourceImage;
  fs << "SourceFlowX" << SourceFlowX;
  fs << "SourceFlowY" << SourceFlowY;

  int SourcePics;
  int SourceStride;
  dr->GetParams(SourcePics, SourceStride);
  fs << "SourcePics" << SourcePics;
  fs << "SourceStride" << SourceStride;

  fs << "IPCL" << ipc->getL();
  fs << "IPCH" << ipc->getH();
  fs << "IPCL2size" << ipc->L2size;
  fs << "IPCapplybandpass" << ipc->applyBandpass;
  fs << "IPCapplywindow" << ipc->applyWindow;
  fs << "IPCwinsize" << ipc->getrows();
}

void LoadDiffrotResultsFromFile(const std::string &path, DiffrotResults *dr, IPCsettings *ipc)
{
  LOG_STARTEND("Loading diffrot results ...", "Diffrot results loaded");

  if (!std::filesystem::exists(path))
  {
    LOG_ERROR("File {} does not exist, cannot load", path);
    return;
  }

  FileStorage fs(path, FileStorage::READ);

  std::vector<double> SourceThetasavg;
  std::vector<double> SourceOmegasXavg;
  std::vector<double> SourceOmegasYavg;
  std::vector<double> SourceShiftsXavg;
  std::vector<double> SourceShiftsYavg;
  fs["SourceThetasavg"] >> SourceThetasavg;
  fs["SourceOmegasXavg"] >> SourceOmegasXavg;
  fs["SourceOmegasYavg"] >> SourceOmegasYavg;
  fs["SourceShiftsXavg"] >> SourceShiftsXavg;
  fs["SourceShiftsYavg"] >> SourceShiftsYavg;
  dr->SetVecs1DRaw(SourceThetasavg, SourceOmegasXavg, SourceOmegasYavg, SourceShiftsXavg, SourceShiftsYavg);

  std::vector<std::vector<double>> SourceShiftsX;
  std::vector<std::vector<double>> SourceShiftsY;
  fs["SourceShiftsX"] >> SourceShiftsX;
  fs["SourceShiftsY"] >> SourceShiftsY;
  dr->SetVecs2DRaw(SourceShiftsX, SourceShiftsY);

  Mat SourceImage;
  Mat SourceFlowX;
  Mat SourceFlowY;
  fs["SourceImage"] >> SourceImage;
  fs["SourceFlowX"] >> SourceFlowX;
  fs["SourceFlowY"] >> SourceFlowY;
  dr->SetMatsRaw(SourceImage, SourceFlowX, SourceFlowY);

  int SourcePics;
  int SourceStride;
  fs["SourcePics"] >> SourcePics;
  fs["SourceStride"] >> SourceStride;
  dr->SetParamsRaw(SourcePics, SourceStride);

  double L, H;
  int winsize, L2size;
  bool applyBandpass, applyWindow;
  fs["IPCL"] >> L;
  fs["IPCH"] >> H;
  fs["IPCL2size"] >> L2size;
  fs["IPCapplybandpass"] >> applyBandpass;
  fs["IPCapplywindow"] >> applyWindow;
  fs["IPCwinsize"] >> winsize;

  if (winsize)
  {
    ipc->setBandpassParameters(L, H);
    ipc->L2size = L2size;
    ipc->applyBandpass = applyBandpass;
    ipc->applyWindow = applyWindow;
    ipc->setSize(winsize, winsize);
  }
  else
    LOG_ERROR("IPC parameters not specified in file!");

  LOG_DEBUG("SourcePics = {}", SourcePics);
  LOG_DEBUG("SourceStride = {}", SourceStride);
  LOG_DEBUG("IPC parameters = {}", std::vector<double>{L, H, (double)L2size, (double)applyBandpass, (double)applyWindow, (double)winsize});

  dr->calculated = true;
}
