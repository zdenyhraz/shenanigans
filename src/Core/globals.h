#pragma once

#include "IPC/IPC.h"
#include "IPC/IterativePhaseCorrelation.h"

class Globals
{
public:
  Globals() { IPC = std::make_unique<IterativePhaseCorrelation>(64, 64, 0.1, 0.6); }

  std::unique_ptr<IPCsettings> IPCset; // obsolete
  std::unique_ptr<IterativePhaseCorrelation> IPC;
};
