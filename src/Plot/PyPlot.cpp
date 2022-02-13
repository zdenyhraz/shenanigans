#include "PyPlot.h"

void Plt::Plot(std::string&& name, Data&& data)
{
  if (not mPlotIds.contains(name))
    mPlotIds[name] = mId++;

  py::eval_file("../script/plot/plot_1d.py", GetScopeData(name, data));
}

py::dict Plt::GetScopeData(const std::string& name, const Data& data)
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
  scope["zlabel"] = data.zlabel;
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
