#pragma once
#include "PlotBase.hpp"

class ImGuiPlot : public PlotBase<ImGuiPlot>
{
  std::vector<PlotData1D> mPlots1D;
  std::vector<PlotData2D> mPlots2D;
  std::mutex mPlotsMutex;

  void RenderInternal();
  void RenderInternal(const PlotData1D& data) const;
  void RenderInternal(const PlotData2D& data) const;
  static ImPlotColormap GetColormap(const std::string& cmap);
  void ClearInternal();
  void Debug();

public:
  void PlotInternal(PlotData1D&& data) override;
  void PlotInternal(PlotData2D&& data) override;
  static void Render() { Singleton<ImGuiPlot>::Get().RenderInternal(); }
  static void Clear() { Singleton<ImGuiPlot>::Get().ClearInternal(); }
};
