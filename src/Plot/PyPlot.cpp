#include "PyPlot.hpp"
#include "Python/Python.hpp"

void PyPlot::Initialize()
try
{
  PROFILE_FUNCTION;
  LOG_DEBUG("Initializing Matplotlib ...");
  Python::Initialize();
  PYTHON_INTERPRETER_GUARD;
  py::module::import("plot.matplotlib_config").attr("init")();
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void PyPlot::Render()
{
  std::scoped_lock lock(mPlotQueueMutex);
  while (not mPlotQueue.empty())
  {
    PlotInternal(mPlotQueue.front());
    mPlotQueue.pop();
  }
}

void PyPlot::PlotInternal(const PlotData& plotdata)
try
{
  PROFILE_FUNCTION;
  PYTHON_INTERPRETER_GUARD;
  py::module::import(fmt::format("plot.{}", plotdata.type).c_str()).attr("plot")(**plotdata.data);
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void PyPlot::ScheldulePlot(const std::string& name, const std::string& type, py::dict&& data)
{
  std::scoped_lock lock(mPlotQueueMutex);
  mPlotQueue.emplace(name, type, std::move(data));
}

i32 PyPlot::GetPlotId(const std::string& name)
{
  std::scoped_lock lock(mPlotIdMutex);
  if (not mPlotIds.contains(name))
    mPlotIds[name] = mPlotId++;

  return mPlotIds[name];
}

void PyPlot::AddDefaultPlotData(const std::string& name, py::dict& plotData)
{
  plotData["id"] = GetPlotId(name);
  plotData["title"] = name;

  if (not plotData.contains("aspectratio"))
    plotData["aspectratio"] = 1;
  if (not plotData.contains("save"))
    plotData["save"] = mSave ? fmt::format("../data/debug/{}.png", name) : "";
}

py::dict PyPlot::GetDefaultPlotData(const std::string& name)
{
  py::dict plotData;
  AddDefaultPlotData(name, plotData);
  return plotData;
}

py::dict PyPlot::GetPlotData(const std::string& name, py::dict&& data)
{
  std::scoped_lock lock(mPlotQueueMutex);
  py::dict plotData = std::move(data);
  AddDefaultPlotData(name, plotData);
  return plotData;
}

py::dict PyPlot::GetPlotData(const std::string& name, PlotData1D&& data)
{
  std::scoped_lock lock(mPlotQueueMutex);
  py::dict plotData = GetDefaultPlotData(name);
  plotData["x"] = not data.x.empty() ? std::move(data.x) : Iota(0., not data.y.empty() ? data.y.size() : data.ys[0].size());
  plotData["y"] = std::move(data.y);
  plotData["y2"] = std::move(data.y2);
  plotData["ys"] = std::move(data.ys);
  plotData["y2s"] = std::move(data.y2s);
  plotData["xlabel"] = std::move(data.xlabel);
  plotData["ylabel"] = std::move(data.ylabel);
  plotData["y2label"] = std::move(data.y2label);
  plotData["label_y"] = std::move(data.label_y);
  plotData["label_y2"] = std::move(data.label_y2);
  plotData["label_ys"] = std::move(data.label_ys);
  plotData["label_y2s"] = std::move(data.label_y2s);
  plotData["color_y"] = std::move(data.color_y);
  plotData["color_y2"] = std::move(data.color_y2);
  plotData["color_ys"] = std::move(data.color_ys);
  plotData["color_y2s"] = std::move(data.color_y2s);
  plotData["linestyle_y"] = std::move(data.linestyle_y);
  plotData["linestyle_y2"] = std::move(data.linestyle_y2);
  plotData["linestyle_ys"] = std::move(data.linestyle_ys);
  plotData["linestyle_y2s"] = std::move(data.linestyle_y2s);
  plotData["log"] = data.log;
  plotData["aspectratio"] = data.aspectratio;
  if (not data.save.empty())
    plotData["save"] = std::move(data.save);
  return plotData;
}

py::dict PyPlot::GetPlotData(const std::string& name, PlotData2D&& data)
{
  std::scoped_lock lock(mPlotQueueMutex);
  py::dict plotData = GetDefaultPlotData(name);
  data.z.convertTo(data.z, CV_32F);
  plotData["z"] = Python::ToNumpy<f32>(data.z);
  plotData["xlabel"] = std::move(data.xlabel);
  plotData["ylabel"] = std::move(data.ylabel);
  plotData["zlabel"] = std::move(data.zlabel);
  plotData["xmin"] = data.xmin;
  plotData["xmax"] = data.xmax;
  plotData["ymin"] = data.ymin;
  plotData["ymax"] = data.ymax;
  plotData["interp"] = data.interp;
  plotData["aspectratio"] = data.aspectratio;
  plotData["cmap"] = std::move(data.cmap);
  if (not data.save.empty())
    plotData["save"] = std::move(data.save);
  return plotData;
}

py::dict PyPlot::GetPlotData(const std::string& name, PlotData3D&& data)
{
  std::scoped_lock lock(mPlotQueueMutex);
  py::dict plotData = GetDefaultPlotData(name);
  data.z.convertTo(data.z, CV_32F);
  plotData["z"] = Python::ToNumpy<f32>(data.z);
  plotData["xlabel"] = std::move(data.xlabel);
  plotData["ylabel"] = std::move(data.ylabel);
  plotData["zlabel"] = std::move(data.zlabel);
  plotData["xmin"] = data.xmin;
  plotData["xmax"] = data.xmax;
  plotData["ymin"] = data.ymin;
  plotData["ymax"] = data.ymax;
  plotData["aspectratio"] = data.aspectratio;
  plotData["cmap"] = std::move(data.cmap);
  if (not data.save.empty())
    plotData["save"] = std::move(data.save);
  return plotData;
}

void PyPlot::Plot(const std::string& name, PlotData1D&& data)
{
  ScheldulePlot(name, "1d", GetPlotData(name, std::move(data)));
}

void PyPlot::Plot(const std::string& name, PlotData2D&& data)
{
  ScheldulePlot(name, "2d", GetPlotData(name, std::move(data)));
}

void PyPlot::PlotSurf(const std::string& name, PlotData3D&& data)
{
  ScheldulePlot(name, "3d", GetPlotData(name, std::move(data)));
}

void PyPlot::Plot(const std::string& name, const std::string& type, py::dict&& data)
{
  ScheldulePlot(name, type, GetPlotData(name, std::move(data)));
}
