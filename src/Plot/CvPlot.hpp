#pragma once
#include "PlotBase.hpp"

class CvPlot : public PlotBase<CvPlot>
{
  static cv::ColormapTypes GetColormap(const std::string& cmap);
  void Debug();

public:
  void RenderInternal(const PlotData1D& data) override;
  void RenderInternal(const PlotData2D& data) override;
};
