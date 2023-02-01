#include "PyPlot.hpp"
#include "Python/Python.hpp"

void PyPlot::InitializeInternal()
try
{
  PROFILE_FUNCTION;
  LOG_DEBUG("Initializing Matplotlib");
  Python::Initialize();
  PYTHON_INTERPRETER_GUARD;
  py::module::import("plot.matplotlib_config").attr("init")();
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void PyPlot::RenderInternal()
{
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

  while (not mPlotsCustom.empty())
  {
    RenderInternal(mPlotsCustom.back().first, std::move(mPlotsCustom.back().second));
    mPlotsCustom.pop_back();
  }
}

void PyPlot::RenderInternal(const PlotData1D& data)
{
  return RenderInternal("1d", GetPythonPlotData(data));
}

void PyPlot::RenderInternal(const PlotData2D& data)
{
  if (data.surf)
    return RenderInternal("3d", GetPythonPlotData(data));

  return RenderInternal("2d", GetPythonPlotData(data));
}

void PyPlot::RenderInternal(const std::string& type, py::dict&& data)
try
{
  PROFILE_FUNCTION;
  PYTHON_INTERPRETER_GUARD;
  py::module::import(fmt::format("plot.{}", type).c_str()).attr("plot")(**data);
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

py::dict PyPlot::GetDefaultPlotData(const std::string& name)
{
  py::dict plotData;
  plotData["name"] = name;
  return plotData;
}

py::dict PyPlot::GetPythonPlotData(const PlotData1D& data)
{
  py::dict plotData = GetDefaultPlotData(data.name);
  plotData["x"] = data.x;
  plotData["ys"] = data.ys;
  plotData["y2s"] = data.y2s;
  plotData["xlabel"] = data.xlabel;
  plotData["ylabel"] = data.ylabel;
  plotData["y2label"] = data.y2label;
  plotData["ylabels"] = data.ylabels;
  plotData["y2labels"] = data.y2labels;
  plotData["ycolors"] = data.ycolors;
  plotData["y2colors"] = data.y2colors;
  plotData["ylinestyles"] = data.ylinestyles;
  plotData["y2linestyles"] = data.y2linestyles;
  plotData["log"] = data.log;
  plotData["aspectratio"] = data.aspectratio;
  plotData["savepath"] = data.savepath;
  return plotData;
}

py::dict PyPlot::GetPythonPlotData(const PlotData2D& data)
{
  py::dict plotData = GetDefaultPlotData(data.name);
  plotData["z"] = Python::ToNumpy<f32>(data.z);
  plotData["xlabel"] = data.xlabel;
  plotData["ylabel"] = data.ylabel;
  plotData["zlabel"] = data.zlabel;
  plotData["xmin"] = data.xmin;
  plotData["xmax"] = data.xmax;
  plotData["ymin"] = data.ymin;
  plotData["ymax"] = data.ymax;
  plotData["interp"] = data.interpolate;
  plotData["aspectratio"] = data.aspectratio;
  plotData["cmap"] = data.cmap;
  plotData["savepath"] = data.savepath;
  return plotData;
}

void PyPlot::ScheduleCustomPlot(const std::string& type, py::dict&& data)
{
  std::scoped_lock lock(mPlotsMutex);
  if (auto it = std::ranges::find_if(mPlotsCustom, [&data](const auto& entry) { return entry.second["name"].template cast<std::string>() == data["name"].cast<std::string>(); });
      it != mPlotsCustom.end())
    it->second = std::move(data);
  else
    mPlotsCustom.emplace_back(type, std::move(data));
}
