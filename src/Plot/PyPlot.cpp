#include "PyPlot.h"

void PyPlot::Initialize()
{
  py::exec(R"(
    import matplotlib.pyplot as plt

    FONT_SCALE = 2.2
    SMALL_SIZE = 8*FONT_SCALE
    MEDIUM_SIZE = 10*FONT_SCALE
    BIGGER_SIZE = 12*FONT_SCALE

    plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
    plt.rc('axes', titlesize=SMALL_SIZE)     # fontsize of the axes title
    plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
    plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
    plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
    plt.rc('legend', fontsize=SMALL_SIZE)    # legend fontsize
    plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title
  )");
}

void PyPlot::Plot(std::string&& name, Data1D&& data)
try
{
  PROFILE_FUNCTION;
  CheckIfPlotExists(name);
  py::eval_file("../script/plot/plot_1d.py", GetScopeData(name, data));
}
catch (const std::exception& e)
{
  LOG_ERROR("PyPlot::Plot error: {}", e.what());
}

void PyPlot::Plot(std::string&& name, Data2D&& data)
try
{
  PROFILE_FUNCTION;
  CheckIfPlotExists(name);
  py::eval_file("../script/plot/plot_2d.py", GetScopeData(name, data));
}
catch (const std::exception& e)
{
  LOG_ERROR("PyPlot::Plot error: {}", e.what());
}

void PyPlot::CheckIfPlotExists(const std::string& name)
{
  if (not mPlotIds.contains(name))
    mPlotIds[name] = mId++;
}

py::dict PyPlot::GetScopeData(const std::string& name, const Data1D& data)
{
  PROFILE_FUNCTION;
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
  scope["aspectratio"] = data.aspectratio;
  scope["title"] = not data.title.empty() ? data.title : name;

  return scope;
}

py::dict PyPlot::GetScopeData(const std::string& name, const Data2D& data)
{
  PROFILE_FUNCTION;
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
  scope["interp"] = data.interp;
  scope["aspectratio"] = data.aspectratio;
  scope["title"] = not data.title.empty() ? data.title : name;

  return scope;
}
