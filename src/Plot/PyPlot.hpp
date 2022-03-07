#pragma once

class PyPlot
{
  // colors: b, g, r, c, m, y, k, w, tab:blue, tab:orange, tab:green, tab:red, tab:purple, tab:brown, tab:pink, tab:gray, tab:olive, tab:cyan
  // for all colors see https://matplotlib.org/stable/gallery/color/named_colors.html
public:
  struct Data1D
  {
    std::vector<f64> x, y, y2;                              // plot data
    std::vector<std::vector<f64>> ys, y2s;                  // plot multi data
    std::string xlabel = "x", ylabel = "y", y2label = "y2"; // plot labels
    std::string label_y = "y", label_y2 = "y2";             // data labels
    std::vector<std::string> label_ys, label_y2s;           // multi data labels
    std::string color_y, color_y2;                          // line colors
    std::vector<std::string> color_ys, color_y2s;           // line colors
    std::string linestyle_y = "-", linestyle_y2 = "-";      // line styles
    std::vector<std::string> linestyle_ys, linestyle_y2s;   // multi line styles
    f64 aspectratio = 1;                                    // aspect ratio
    std::string save;                                       // save path
    std::string title;                                      // plot title
  };

  struct Data2D
  {
    cv::Mat z;
    f64 xmin = 0, xmax = 1, ymin = 0, ymax = 1;
    std::string xlabel = "x", ylabel = "y", zlabel = "z";
    bool interp = false;
    f64 aspectratio = 1;
    std::string cmap = "jet";
    std::string save;
    std::string title;
  };

  struct Data3D
  {
    cv::Mat z;
    f64 xmin = 0, xmax = 1, ymin = 0, ymax = 1;
    std::string xlabel = "x", ylabel = "y", zlabel = "z";
    f64 aspectratio = 1;
    std::string cmap = "jet";
    u32 rstride = 0, cstride = 0;
    std::string save;
    std::string title;
  };

  static void Plot(const std::string& name, const Data1D& data) { Get().PlotInternal(name, data); }
  static void Plot(const std::string& name, const Data2D& data) { Get().PlotInternal(name, data); }
  static void PlotSurf(const std::string& name, const Data3D& data) { Get().PlotInternal(name, data); }

private:
  std::map<std::string, u32> mPlotIds;
  u32 mId = 0;

  PyPlot();

  static PyPlot& Get()
  {
    static PyPlot plt;
    return plt;
  }

  void CheckIfPlotExists(const std::string& name);
  py::dict GetScopeData(const std::string& name, const Data1D& data);
  py::dict GetScopeData(const std::string& name, const Data2D& data);
  py::dict GetScopeData(const std::string& name, const Data3D& data);
  void PlotInternal(const std::string& name, const Data1D& data);
  void PlotInternal(const std::string& name, const Data2D& data);
  void PlotInternal(const std::string& name, const Data3D& data);
};
