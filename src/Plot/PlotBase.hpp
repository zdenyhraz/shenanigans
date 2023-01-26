#pragma once
#include "PlotData1D.hpp"
#include "PlotData2D.hpp"
#include "Utils/Singleton.hpp"

template <class T>
class PlotBase
{
  virtual void PlotInternal(PlotData1D&& data) = 0;
  virtual void PlotInternal(PlotData2D&& data) = 0;

public:
  static void Plot(PlotData1D&& data) { return Singleton<T>::Get().PlotInternal(std::move(data)); }
  static void Plot(PlotData2D&& data) { return Singleton<T>::Get().PlotInternal(std::move(data)); }
  static void Plot(const std::string& name, const cv::Mat& z) { return Singleton<T>::Get().PlotInternal({.name = name, .z = z}); }
};
