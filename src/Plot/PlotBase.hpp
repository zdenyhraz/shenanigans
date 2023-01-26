#pragma once
#include "PlotData1D.hpp"
#include "PlotData2D.hpp"
#include "Utils/Singleton.hpp"

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

  void SchedulePlot(PlotData1D&& data)
  {
    std::scoped_lock lock(mPlotsMutex);
    if (auto it = std::ranges::find_if(mPlots1D, [&data](const auto& entry) { return entry.name == data.name; }); it != mPlots1D.end())
      *it = std::move(data);
    else
      mPlots1D.emplace_back(std::move(data));
  }

  void SchedulePlot(PlotData2D&& data)
  {
    std::scoped_lock lock(mPlotsMutex);
    if (auto it = std::ranges::find_if(mPlots2D, [&data](const auto& entry) { return entry.name == data.name; }); it != mPlots2D.end())
      *it = std::move(data);
    else
      mPlots2D.emplace_back(std::move(data));
  }

protected:
  std::vector<PlotData1D> mPlots1D;
  std::vector<PlotData2D> mPlots2D;
  std::mutex mPlotsMutex;
  bool mSave = false;

public:
  static void Initialize() { Singleton<T>::Get().InitializeInternal(); }
  static void Render() { Singleton<T>::Get().RenderInternal(); }
  static void Clear() { Singleton<T>::Get().ClearInternal(); }
  static void SetSave(bool save) { Singleton<T>::Get().mSave = save; }
  static void Plot(PlotData1D&& data) { Singleton<T>::Get().SchedulePlot(std::move(data)); }
  static void Plot(PlotData2D&& data) { Singleton<T>::Get().SchedulePlot(std::move(data)); }
  static void Plot(const std::string& name, const cv::Mat& z) { Singleton<T>::Get().SchedulePlot({.name = name, .z = z}); }
};
