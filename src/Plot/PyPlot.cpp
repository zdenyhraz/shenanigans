#include "PyPlot.hpp"
#include "Python/Python.hpp"

void PyPlot::Initialize()
try
{
  PROFILE_FUNCTION;
  LOG_DEBUG("Initializing Matplotlib ...");
  Python::Initialize();
  py::module::import("plot.init").attr("init")();
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void PyPlot::Render()
{
  PROFILE_FUNCTION;
  std::scoped_lock lock(mMutex);

  while (not mPlotQueue.empty())
  {
    PlotInternal(mPlotQueue.front());
    mPlotQueue.pop();
  }
}

void PyPlot::ScheldulePlot(const std::string& name, const std::string& type, const py::dict& data)
{
  std::scoped_lock lock(mMutex);
  mPlotQueue.emplace(name, type, data);
}

void PyPlot::PlotInternal(const PlotData& plotdata)
try
{
  PROFILE_FUNCTION;
  py::module::import(fmt::format("plot.{}", plotdata.type).c_str()).attr("plot")(**plotdata.data);
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

i32 PyPlot::GetPlotId(const std::string& name)
{
  if (not mPlotIds.contains(name))
    mPlotIds[name] = mId++;

  return mPlotIds[name];
}

void PyPlot::AddDefaultScopeData(const std::string& name, py::dict& scope)
{
  scope["id"] = GetPlotId(name);
  scope["title"] = name;

  if (not scope.contains("aspectratio"))
    scope["aspectratio"] = 1;
  if (not scope.contains("save"))
    scope["save"] = "";
}

py::dict PyPlot::GetScopeData(const std::string& name, const py::dict& data)
{
  std::scoped_lock lock(mMutex);

  py::dict scope = data;
  AddDefaultScopeData(name, scope);
  return scope;
}

py::dict PyPlot::GetScopeData(const std::string& name, const PlotData1D& data)
{
  PROFILE_FUNCTION;
  std::scoped_lock lock(mMutex);

  py::dict scope;
  AddDefaultScopeData(name, scope);
  scope["x"] = not data.x.empty() ? data.x : Iota(0., not data.y.empty() ? data.y.size() : data.ys[0].size());
  scope["y"] = data.y;
  scope["y2"] = data.y2;
  scope["ys"] = data.ys;
  scope["y2s"] = data.y2s;
  scope["xlabel"] = data.xlabel;
  scope["ylabel"] = data.ylabel;
  scope["y2label"] = data.y2label;
  scope["label_y"] = data.label_y;
  scope["label_y2"] = data.label_y2;
  scope["label_ys"] = data.label_ys;
  scope["label_y2s"] = data.label_y2s;
  scope["color_y"] = data.color_y;
  scope["color_y2"] = data.color_y2;
  scope["color_ys"] = data.color_ys;
  scope["color_y2s"] = data.color_y2s;
  scope["linestyle_y"] = data.linestyle_y;
  scope["linestyle_y2"] = data.linestyle_y2;
  scope["linestyle_ys"] = data.linestyle_ys;
  scope["linestyle_y2s"] = data.linestyle_y2s;
  scope["log"] = data.log;
  scope["aspectratio"] = data.aspectratio;
  scope["save"] = data.save;
  return scope;
}

py::dict PyPlot::GetScopeData(const std::string& name, const PlotData2D& data)
{
  PROFILE_FUNCTION;
  std::scoped_lock lock(mMutex);

  cv::Mat mz = data.z.clone();
  mz.convertTo(mz, CV_32F);
  auto z = Zerovect2(mz.rows, mz.cols, 0.0f);
  for (i32 r = 0; r < mz.rows; ++r)
    for (i32 c = 0; c < mz.cols; ++c)
      z[r][c] = mz.at<f32>(r, c);

  py::dict scope;
  AddDefaultScopeData(name, scope);
  scope["z"] = z;
  scope["xlabel"] = data.xlabel;
  scope["ylabel"] = data.ylabel;
  scope["zlabel"] = data.zlabel;
  scope["xmin"] = data.xmin;
  scope["xmax"] = data.xmax;
  scope["ymin"] = data.ymin;
  scope["ymax"] = data.ymax;
  scope["interp"] = data.interp;
  scope["aspectratio"] = data.aspectratio;
  scope["cmap"] = data.cmap;
  scope["save"] = data.save;
  return scope;
}

py::dict PyPlot::GetScopeData(const std::string& name, const PlotData3D& data)
{
  PROFILE_FUNCTION;
  std::scoped_lock lock(mMutex);

  cv::Mat mz = data.z.clone();
  mz.convertTo(mz, CV_32F);
  auto z = Zerovect2(mz.rows, mz.cols, 0.0f);
  for (i32 r = 0; r < mz.rows; ++r)
    for (i32 c = 0; c < mz.cols; ++c)
      z[r][c] = mz.at<f32>(r, c);

  py::dict scope;
  AddDefaultScopeData(name, scope);
  scope["z"] = z;
  scope["xlabel"] = data.xlabel;
  scope["ylabel"] = data.ylabel;
  scope["zlabel"] = data.zlabel;
  scope["xmin"] = data.xmin;
  scope["xmax"] = data.xmax;
  scope["ymin"] = data.ymin;
  scope["ymax"] = data.ymax;
  scope["aspectratio"] = data.aspectratio;
  scope["cmap"] = data.cmap;
  scope["save"] = data.save;
  return scope;
}

void PyPlot::Plot(const std::string& name, const PlotData1D& data)
{
  ScheldulePlot(name, "1d", GetScopeData(name, data));
}

void PyPlot::Plot(const std::string& name, const PlotData2D& data)
{
  ScheldulePlot(name, "2d", GetScopeData(name, data));
}

void PyPlot::PlotSurf(const std::string& name, const PlotData3D& data)
{
  ScheldulePlot(name, "3d", GetScopeData(name, data));
}

void PyPlot::Plot(const std::string& name, const std::string& type, const py::dict& data)
{
  ScheldulePlot(name, type, GetScopeData(name, data));
}

void PyPlot::SavePlot(const std::string& path, const std::string& name, const PlotData1D& data)
{
  auto scope = GetScopeData(name, data);
  scope["save"] = path;
  ScheldulePlot(name, "1d", scope);
}

void PyPlot::SavePlot(const std::string& path, const std::string& name, const PlotData2D& data)
{
  auto scope = GetScopeData(name, data);
  scope["save"] = path;
  ScheldulePlot(name, "2d", scope);
}

void PyPlot::SavePlotSurf(const std::string& path, const std::string& name, const PlotData3D& data)
{
  auto scope = GetScopeData(name, data);
  scope["save"] = path;
  ScheldulePlot(name, "3d", scope);
}

void PyPlot::SavePlot(const std::string& path, const std::string& name, const std::string& type, const py::dict& data)
{
  auto scope = GetScopeData(name, data);
  scope["save"] = path;
  ScheldulePlot(name, type, scope);
}
