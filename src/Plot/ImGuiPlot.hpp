#pragma once
#include "PlotData.hpp"
#include "Utils/Singleton.hpp"

class ImGuiPlot : public Singleton<ImGuiPlot>
{
public:
  struct PlotData
  {
    PlotData() = default;

    PlotData(PlotData1D&& data1d)
    {
      if (data1d.ylabels.size() < data1d.ys.size())
      {
        const auto origsize = data1d.ylabels.size();
        data1d.ylabels.resize(data1d.ys.size());
        for (usize i = origsize; i < data1d.ys.size(); ++i)
          data1d.ylabels[i] = fmt::format("y{}", i);
      }

      if (data1d.y2labels.size() < data1d.y2s.size())
      {
        const auto origsize = data1d.y2labels.size();
        data1d.y2labels.resize(data1d.y2s.size());
        for (usize i = origsize; i < data1d.y2s.size(); ++i)
          data1d.y2labels[i] = fmt::format("y2-{}", i);
      }

      data = std::move(data1d);
    }

    PlotData(PlotData2D&& data2d)
    {
      const auto [zmin, zmax] = MinMax(data2d.z);
      data2d.zmin = zmin;
      data2d.zmax = zmax;
      data2d.z.convertTo(data2d.z, CV_32F);
      cv::resize(data2d.z, data2d.z, cv::Size(501, 501));
      data = std::move(data2d);
    }

    std::variant<PlotData1D, PlotData2D> data;
  };

  void Render();
  void Clear();

  template <typename T>
  void Plot(const std::string& name, T&& data)
  {
    auto plotName = fmt::format("##{}", name);
    std::scoped_lock lock(mPlotsMutex);
    if (auto it = std::find_if(mPlots.begin(), mPlots.end(), [&plotName](const auto& entry) { return entry.first == plotName; }); it != mPlots.end())
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
