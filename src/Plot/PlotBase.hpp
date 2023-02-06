#pragma once
#include "PlotData1D.hpp"
#include "PlotData2D.hpp"
#include "Utils/Singleton.hpp"
#include "Utils/Colormap.hpp"

class ImGuiPlot;
class PyPlot;
class CvPlot;

template <class T>
class PlotBase
{
  virtual void InitializeInternal() {}
  virtual void RenderInternal() = 0;
  virtual void RenderInternal(const PlotData1D& data) = 0;
  virtual void RenderInternal(const PlotData2D& data) = 0;

  void ClearInternal()
  {
    std::scoped_lock lock(mPlotsMutex);
    mPlots1D.clear();
    mPlots2D.clear();
  }

  template <class PlotData>
  void SchedulePlot(PlotData&& data, std::vector<PlotData>& vec)
  {
    PROFILE_FUNCTION;
    std::scoped_lock lock(mPlotsMutex);
    if (auto it = std::ranges::find_if(vec, [&data](const auto& entry) { return entry.name == data.name; }); it != vec.end())
      *it = std::forward<PlotData>(data);
    else
      vec.emplace_back(std::forward<PlotData>(data));
  }

protected:
  std::vector<PlotData1D> mPlots1D;
  std::vector<PlotData2D> mPlots2D;
  std::mutex mPlotsMutex;
  bool mSave = false;

public:
  static void Initialize()
  {
    PROFILE_FUNCTION;
    Singleton<T>::Get().InitializeInternal();
  }

  static void Render()
  {
    PROFILE_FUNCTION;
    Singleton<T>::Get().RenderInternal();
  }

  static void Clear()
  {
    PROFILE_FUNCTION;
    Singleton<T>::Get().ClearInternal();
  }

  static void Debug()
  {
    static constexpr usize n = 501;
    const f64 y1A = Random::Rand<f64>(0.5, 1);
    const f64 y2A = Random::Rand<f64>(0.5, 1);
    const f64 y1F = Random::Rand<f64>(2, 5);
    const f64 y2F = Random::Rand<f64>(2, 5);
    std::vector<f64> x(n);
    std::vector<f64> y1(n);
    std::vector<f64> y2(n);
    for (usize i = 0; i < n; ++i)
    {
      x[i] = static_cast<f64>(3 * i) / (n - 1);
      y1[i] = y1A * std::sin(y1F * std::numbers::pi * x[i]);
      y2[i] = y2A * std::cos(y2F * std::numbers::pi * x[i]);
    }

    Plot({.name = fmt::format("debug1d({})", Singleton<T>::Get().mPlots1D.size()), .x = x, .ys = {y1, y2}, .ylabels = {"Asin(fx)", "Acos(fx)"}});

    Plot({.name = fmt::format("debug2d({})", Singleton<T>::Get().mPlots2D.size()),
        .z = Random::Rand<f32>(0, 1) * Gaussian<f32>(n, Random::Rand<f32>(0, 0.5) * n),
        .xmin = -3,
        .xmax = 3,
        .ymin = -5,
        .ymax = 5,
        .xlabel = "xlabeeel",
        .ylabel = "ylabeeel",
        .zlabel = "zlabeeel"});
  }

  static void SetSave(bool save) { Singleton<T>::Get().mSave = save; }

  static void Plot(PlotData1D&& data)
  {
    PROFILE_FUNCTION;
    if (data.x.empty())
      data.x = Iota(0., data.ys[0].size());

    Singleton<T>::Get().SchedulePlot(std::move(data), Singleton<T>::Get().mPlots1D);
  }

  static void Plot(PlotData2D&& data)
  {
    PROFILE_FUNCTION;
    if (data.z.empty())
      throw std::invalid_argument("Unable to plot empty data");
    if (not data.z.isContinuous())
      throw std::invalid_argument("Unable to plot non-continuous data");
    if (data.z.channels() != 1 and data.z.channels() != 3)
      throw std::invalid_argument(fmt::format("Unable to plot {}-channel data", data.z.channels()));

    data.z = data.z.clone();
    data.aspectratio = static_cast<f64>(data.z.cols) / data.z.rows;
    if (data.xmin == PlotData2D::Default or data.xmax == PlotData2D::Default)
    {
      data.xmin = 0;
      data.xmax = data.z.cols - 1;
    }
    if (data.ymin == PlotData2D::Default or data.ymax == PlotData2D::Default)
    {
      data.ymin = data.z.rows - 1;
      data.ymax = 0;
    }
    if (data.savepath.empty() and Singleton<T>::Get().mSave)
      data.savepath = fmt::format("../data/debug/{}.png", data.name);

    if (data.z.channels() == 3)
    {
      cv::normalize(data.z, data.z, 0, 255, cv::NORM_MINMAX);
      data.z.convertTo(data.z, CV_8UC3);
      // data.colorbar = false;
      data.zmin = 0;
      data.zmax = 1;
    }

    if (data.z.channels() == 1)
    {
      data.z.convertTo(data.z, CV_32F);
      const auto [zmin, zmax] = MinMax(data.z);
      data.zmin = zmin;
      data.zmax = zmax;
      if constexpr (std::is_same_v<T, ImGuiPlot> or std::is_same_v<T, CvPlot>)
      {
        cv::normalize(data.z, data.z, 0, 255, cv::NORM_MINMAX);
        data.z.convertTo(data.z, CV_8U);
        cv::Mat cmap;
        cv::applyColorMap(data.z, cmap, GetColormap(data.cmap));
        data.z = cmap;
      }
    }

    Singleton<T>::Get().SchedulePlot(std::move(data), Singleton<T>::Get().mPlots2D);
  }

  static void Plot(const std::string& name, const cv::Mat& z) { Plot({.name = name, .z = z}); }
};
