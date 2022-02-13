#include "PyPlot.h"

void PyPlot::Plot(std::string&& name, Data1D&& data)
{
  CheckIfPlotExists(name);
  py::eval_file("../script/plot/plot_1d.py", GetScopeData(name, data));
}

void PyPlot::Plot(std::string&& name, Data2D&& data)
{
  CheckIfPlotExists(name);
  py::eval_file("../script/plot/plot_2d.py", GetScopeData(name, data));
}

void PyPlot::CheckIfPlotExists(const std::string& name)
{
  if (not mPlotIds.contains(name))
    mPlotIds[name] = mId++;
}

py::dict PyPlot::GetScopeData(const std::string& name, const Data1D& data)
{
  py::dict scope;
  scope["id"] = mPlotIds[name];
  scope["x"] = data.x;
  scope["y"] = data.y;
  scope["y2"] = data.y2;
  scope["ys"] = data.ys;
  scope["y2s"] = data.y2s;
  scope["xlabel"] = data.xlabel;
  scope["ylabel"] = data.ylabel;
  scope["y2label"] = data.y2label;
  scope["title"] = data.title;
  scope["label_y"] = data.label_y;
  scope["label_y2"] = data.label_y2;
  scope["label_ys"] = data.label_ys;
  scope["label_y2s"] = data.label_y2s;
  scope["linestyle_y"] = data.linestyle_y;
  scope["linestyle_y2"] = data.linestyle_y2;
  scope["linestyle_ys"] = data.linestyle_ys;
  scope["linestyle_y2s"] = data.linestyle_y2s;

  return scope;
}

py::dict PyPlot::GetScopeData(const std::string& name, const Data2D& data)
{
  auto z = zerovect2(data.z.rows, data.z.cols, 0.0f);
  for (i32 r = 0; r < data.z.rows; ++r)
    for (i32 c = 0; c < data.z.cols; ++c)
      z[r][c] = data.z.at<f32>(r, c);

  py::dict scope;
  scope["id"] = mPlotIds[name];
  scope["z"] = z;
  scope["xlabel"] = data.xlabel;
  scope["ylabel"] = data.ylabel;
  scope["zlabel"] = data.zlabel;
  scope["xmin"] = data.xmin;
  scope["xmax"] = data.xmax;
  scope["ymin"] = data.ymin;
  scope["ymax"] = data.ymax;
  scope["title"] = data.title;

  return scope;
}
