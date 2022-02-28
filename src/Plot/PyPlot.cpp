#include "PyPlot.h"

void PyPlot::Initialize()
{
  std::call_once(mInitialized, []() {
    PROFILE_FUNCTION;
    py::eval_file("../script/plot/plot_init.py");
  });
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
  if (data.x.empty())
  {
    std::vector<f64> x(not data.y.empty() ? data.y.size() : data.ys.size());
    std::iota(x.begin(), x.end(), 0);
  }
  scope["id"] = mPlotIds[name];
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
  scope["aspectratio"] = data.aspectratio;
  scope["save"] = data.save;
  scope["title"] = not data.title.empty() ? data.title : name;

  return scope;
}

py::dict PyPlot::GetScopeData(const std::string& name, const Data2D& data)
{
  PROFILE_FUNCTION;
  cv::Mat mz = data.z.clone();
  mz.convertTo(mz, CV_32F);
  auto z = Zerovect2(mz.rows, mz.cols, 0.0f);
  for (i32 r = 0; r < mz.rows; ++r)
    for (i32 c = 0; c < mz.cols; ++c)
      z[r][c] = mz.at<f32>(r, c);

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
  scope["cmap"] = data.cmap;
  scope["save"] = data.save;
  scope["title"] = not data.title.empty() ? data.title : name;

  return scope;
}
