#pragma once

class ImGuiPlot
{
public:
  struct PlotData1D
  {
    std::vector<f64> x;
    std::vector<std::vector<f64>> ys;
    std::vector<std::vector<f64>> y2s;
    std::vector<std::string> ylabels;
    std::vector<std::string> y2labels;
  };

  struct PlotData2D
  {
    cv::Mat z;
    f32 xmin = 0;
    f32 xmax = 1;
    f32 ymin = 0;
    f32 ymax = 1;
    f32 zmin = 0;
    f32 zmax = 1;
  };

  struct PlotData
  {
    PlotData(PlotData1D&& data1d) : data(std::move(data1d)) {}

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

  static void Render();
  static void Clear();

  template <typename T>
  static void Plot(const std::string& name, T&& data)
  {
    std::scoped_lock lock(mPlotsMutex);
    const auto plotName = fmt::format("##{}", name);
    if (auto it = std::find_if(mPlots.begin(), mPlots.end(), [&plotName](const auto& entry) { return entry.first == plotName; }); it != mPlots.end())
      it->second = std::forward<T>(data);
    else
      mPlots.emplace_back(plotName, std::forward<T>(data));
  }

private:
  inline static std::vector<std::pair<std::string, PlotData>> mPlots;
  inline static std::mutex mPlotsMutex;

  static void RenderPlot(const std::string& name, const PlotData& data);
  static void RenderPlot1D(const std::string& name, const PlotData1D& data);
  static void RenderPlot2D(const std::string& name, const PlotData2D& data);
  static void Debug();
  static void Debug1D();
  static void Debug2D();
};
