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
  };

  struct PlotData
  {
    std::string name;
    std::variant<PlotData1D, PlotData2D> data;
  };

  static void Render();
  static void Clear();

  template <typename Data>
  static void Plot(const std::string& name, Data&& data)
  {
    std::scoped_lock lock(mPlotsMutex);
    mPlots[name] = {.name = name, .data = std::move(data)};
  }

private:
  inline static std::unordered_map<std::string, PlotData> mPlots;
  inline static std::mutex mPlotsMutex;

  static void RenderPlot(const PlotData& data);
  static void RenderPlot1D(const std::string& name, const PlotData1D& data);
  static void RenderPlot2D(const std::string& name, const PlotData2D& data);
  static void Debug();
  static void Debug1D();
  static void Debug2D();
};
