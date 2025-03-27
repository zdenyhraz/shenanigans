#pragma once
#include "PlotData.hpp"
#include "Utils/Singleton.hpp"
#include "Utils/Colormap.hpp"
#include "Math/Random.hpp"
#include "Math/Functions.hpp"
#include "Utils/Filesystem.hpp"

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

  template <class PlotDataND>
  void SchedulePlot(PlotDataND&& data, std::vector<PlotDataND>& vec)
  {
    PROFILE_FUNCTION;
    std::scoped_lock lock(mPlotsMutex);
    if (auto it = std::ranges::find_if(vec, [&data](const auto& entry) { return entry.name == data.name; }); it != vec.end())
      *it = std::forward<PlotDataND>(data);
    else
      vec.emplace_back(std::forward<PlotDataND>(data));
  }

protected:
  std::vector<PlotData1D> mPlots1D;
  std::vector<PlotData2D> mPlots2D;
  std::mutex mPlotsMutex;
  bool mSave = false;
  bool mShouldPlot = true;
  size_t mWindowCount = 1;

public:
  enum PlotLocation : size_t
  {
    Left,
    Right
  };

  static bool ShouldPlot() { return Singleton<T>::Get().mShouldPlot; }

  static void SetWindowCount(size_t windowCount) { Singleton<T>::Get().mWindowCount = windowCount; }

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
    static constexpr size_t n = 501;
    const double y1A = Random::Rand<double>(0.5, 1);
    const double y2A = Random::Rand<double>(0.5, 1);
    const double y1F = Random::Rand<double>(2, 5);
    const double y2F = Random::Rand<double>(2, 5);
    std::vector<double> x(n);
    std::vector<double> y1(n);
    std::vector<double> y2(n);
    for (size_t i = 0; i < n; ++i)
    {
      x[i] = static_cast<double>(3 * i) / (n - 1);
      y1[i] = y1A * std::sin(y1F * std::numbers::pi * x[i]);
      y2[i] = y2A * std::cos(y2F * std::numbers::pi * x[i]);
    }

    Plot({.name = fmt::format("debug1d({})", Singleton<T>::Get().mPlots1D.size()), .x = x, .ys = {y1, y2}, .ylabels = {"Asin(fx)", "Acos(fx)"}});

    Plot({.name = fmt::format("debug2d({})", Singleton<T>::Get().mPlots2D.size()),
        .z = Random::Rand<float>(0, 1) * Gaussian<float>(n, Random::Rand<float>(0, 0.5) * n),
        .xmin = -3,
        .xmax = 3,
        .ymin = -5,
        .ymax = 5,
        .xlabel = "xlabeeel",
        .ylabel = "ylabeeel",
        .zlabel = "zlabeeel"});
  }

  static void SetSave(bool save) { Singleton<T>::Get().mSave = save; }

  static void SetPlot(bool shouldPlot) { Singleton<T>::Get().mShouldPlot = shouldPlot; }

  static void Plot(PlotData1D&& data)
  {
    PROFILE_FUNCTION;
    if (not ShouldPlot())
      return;
    if (data.x.empty())
      data.x = Iota(0., data.ys[0].size());

    Singleton<T>::Get().SchedulePlot(std::move(data), Singleton<T>::Get().mPlots1D);
  }

  static void Plot(PlotData2D&& data)
  {
    PROFILE_FUNCTION;
    if (not ShouldPlot())
      return;
    if (data.z.empty())
      throw std::invalid_argument("Unable to plot empty data");
    if (not data.z.isContinuous())
      data.z = data.z.clone();
    if (not data.z.isContinuous())
      throw std::invalid_argument("Unable to plot non-continuous data");
    if (data.z.channels() != 1 and data.z.channels() != 3)
      throw std::invalid_argument(fmt::format("Unable to plot {}-channel data", data.z.channels()));

    data.z = data.z.clone();
    data.aspectratio = static_cast<double>(data.z.cols) / data.z.rows;
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
      data.savepath = GetProjectPath(fmt::format("data/debug/{}.png", data.name)).string();

    if constexpr (std::is_same_v<T, ImGuiPlot>)
    {
      if (data.z.rows > 8000 or data.z.cols > 8000)
      {
        static constexpr int maxSize = 5000;
        const float aspect = static_cast<float>(data.z.cols) / data.z.rows;
        int targetWidth = std::min(maxSize, data.z.cols);
        int targetHeight = targetWidth / aspect;

        // LOG_WARNING("Image '{}' is too big for ImGuiPlot ({}x{}), downsizing to {}x{}", data.name, data.z.cols, data.z.rows, targetWidth, targetHeight);
        cv::resize(data.z, data.z, cv::Size(targetWidth, targetHeight));
      }
    }

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

  static void Plot(const std::string& name, const cv::Mat& z, PlotLocation loc = Left)
  {
    if (not ShouldPlot())
      return;
    Plot({.name = name, .location = loc, .z = z});
  }

  static void PlotLeft(const std::string& name, const cv::Mat& z)
  {
    if (not ShouldPlot())
      return;
    Plot({.name = name, .location = Left, .z = z});
  }

  static void PlotRight(const std::string& name, const cv::Mat& z)
  {
    if (not ShouldPlot())
      return;
    Plot({.name = name, .location = Right, .z = z});
  }
};
