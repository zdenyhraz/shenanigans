#pragma once
#include "PlotData1D.hpp"
#include "PlotData2D.hpp"
#include "Utils/Singleton.hpp"

template <class T>
class PlotBase
{
  virtual void InitializeInternal() {}
  virtual void RenderInternal()
  {
    PROFILE_FUNCTION;
    std::scoped_lock lock(mPlotsMutex);

    while (not mPlots1D.empty())
    {
      RenderInternal(mPlots1D.back());
      mPlots1D.pop_back();
    }

    while (not mPlots2D.empty())
    {
      RenderInternal(mPlots2D.back());
      mPlots2D.pop_back();
    }
  }
  virtual void RenderInternal(const PlotData1D& data) = 0;
  virtual void RenderInternal(const PlotData2D& data) = 0;

  void ClearInternal()
  {
    std::scoped_lock lock(mPlotsMutex);
    mPlots1D.clear();
    mPlots2D.clear();
  }

  template <class T>
  void SchedulePlot(T&& data, std::vector<T>& vec)
  {
    std::scoped_lock lock(mPlotsMutex);
    if (auto it = std::ranges::find_if(vec, [&data](const auto& entry) { return entry.name == data.name; }); it != vec.end())
      *it = std::move(data);
    else
      vec.emplace_back(std::move(data));
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

  static void SetSave(bool save) { Singleton<T>::Get().mSave = save; }

  static void Plot(PlotData1D&& data)
  {
    if (data.x.empty())
      data.x = Iota(0., data.ys[0].size());

    Singleton<T>::Get().SchedulePlot(std::move(data), Singleton<T>::Get().mPlots1D);
  }

  static void Plot(PlotData2D&& data)
  {
    if (data.z.channels() > 1)
      throw std::invalid_argument("Multichannel image plots not supported");

    if (data.savepath.empty() and Singleton<T>::Get().mSave)
      data.savepath = fmt::format("../data/debug/{}.png", data.name);

    data.z.convertTo(data.z, CV_32F);
    const auto [zmin, zmax] = MinMax(data.z);
    data.zmin = zmin;
    data.zmax = zmax;
    data.aspectratio = static_cast<f64>(data.z.cols) / data.z.rows;
    Singleton<T>::Get().SchedulePlot(std::move(data), Singleton<T>::Get().mPlots2D);
  }

  static void Plot(const std::string& name, const cv::Mat& z) { Plot({.name = name, .z = z}); }
};
