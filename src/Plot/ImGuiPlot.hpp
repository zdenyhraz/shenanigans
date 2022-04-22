#pragma once
#include "PlotData.hpp"
#include "Utils/Singleton.hpp"

class ImGuiPlot : public Singleton<ImGuiPlot>
{
public:
  struct PlotData
  {
    PlotData() = default;
    PlotData(PlotData1D&& data1d);
    PlotData(PlotData2D&& data2d);

    std::variant<PlotData1D, PlotData2D> data;
  };

  void Render();
  void Clear();

  template <typename T>
  void Plot(const std::string& name, T&& data)
  {
    auto plotName = fmt::format("##{}", name);
    std::scoped_lock lock(mPlotsMutex);
    if (auto it = std::ranges::find_if(mPlots, [&plotName](const auto& entry) { return entry.first == plotName; }); it != mPlots.end())
      it->second = std::forward<T>(data);
    else
      mPlots.emplace_back(std::move(plotName), std::forward<T>(data));
  }

private:
  std::vector<std::pair<std::string, PlotData>> mPlots;
  std::mutex mPlotsMutex;

  void RenderPlot(const std::string& name, const PlotData& data) const;
  void RenderPlot1D(const std::string& name, const PlotData1D& data) const;
  void RenderPlot2D(const std::string& name, const PlotData2D& data) const;
  void Debug();
};
