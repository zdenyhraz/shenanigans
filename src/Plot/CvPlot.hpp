#pragma once
#include "PlotBase.hpp"

class CvPlot : public PlotBase<CvPlot>
{
public:
  void RenderInternal() override;
  void RenderInternal(const PlotData1D& data) override;
  void RenderInternal(const PlotData2D& data) override;
};
