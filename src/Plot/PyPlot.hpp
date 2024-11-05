#pragma once
#include "PlotBase.hpp"

class PyPlot : public PlotBase<PyPlot>
{
  static py::dict GetDefaultPlotData(const std::string& name);
  py::dict GetPythonPlotData(const PlotData1D& data);
  py::dict GetPythonPlotData(const PlotData2D& data);
  void ScheduleCustomPlot(const std::string& type, py::dict&& data);

  std::vector<std::pair<std::string, py::dict>> mPlotsCustom;

public:
  void InitializeInternal() override;
  void RenderInternal() override;
  void RenderInternal(const PlotData1D& data) override;
  void RenderInternal(const PlotData2D& data) override;
  static void RenderInternal(const std::string& type, py::dict&& data);

  static void PlotCustom(const std::string& type, py::dict&& data) { Singleton<PyPlot>::Get().ScheduleCustomPlot(type, std::move(data)); }
};
