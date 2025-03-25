#pragma once
#include "ImageRegistrationDataset.hpp"

class IPC;

class IPCMeasure
{
  static constexpr double mQuanT = 0.95;

public:
  static void MeasureAccuracy(const IPC& ipc, const IPC& ipcopt, const std::string& path);
};
