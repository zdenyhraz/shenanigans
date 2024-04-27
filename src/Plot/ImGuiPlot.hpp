#pragma once

#ifdef GLAPI
  #include "PlotBase.hpp"

class ImGuiPlot : public PlotBase<ImGuiPlot>
{
  static ImPlotColormap GetColormap(const std::string& cmap);

public:
  void RenderInternal() override;
  void RenderInternal(const PlotData1D& data) override;
  void RenderInternal(const PlotData2D& data) override;
};
#endif
