#pragma once

struct IPCMeasureParameters
{
  std::string imagePath = "../debug/ipcopt/shape.png";
  i32 iters = 101;
};

class IPCMeasureWindow
{
public:
  static void Initialize();
  static void Render();

private:
  inline static IPCMeasureParameters mParameters;
  inline static cv::Mat mImage;
};
