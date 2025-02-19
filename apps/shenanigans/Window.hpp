#pragma once
#include "ImageRegistration/IPC.hpp"

class Window
{
protected:
  // shared window data
  inline static IPC mIPC;
  inline static IPC mIPCOptimized;

public:
  virtual ~Window() = default;
  virtual void Initialize() {}
  virtual void Render() = 0;
};
