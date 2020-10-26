#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "IPC/IterativePhaseCorrelation.h"

class Globals
{
public:
  Globals() { IPC = std::make_unique<IterativePhaseCorrelation>(64, 64, 1, 200); }

  std::unique_ptr<IPCsettings> IPCset; // obsolete
  std::unique_ptr<IterativePhaseCorrelation> IPC;
};
