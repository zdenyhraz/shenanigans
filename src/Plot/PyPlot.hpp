#pragma once

class PyPlot
{
  // colors: b, g, r, c, m, y, k, w, tab:blue, tab:orange, tab:green, tab:red, tab:purple, tab:brown, tab:pink, tab:gray, tab:olive, tab:cyan
  // for all colors see https://matplotlib.org/stable/gallery/color/named_colors.html
  // python interpreter calls have to be from the main thread, which is the reason for async plotting (render queues)

  struct PlotData
  {
    std::string name;
    std::string type;
    py::dict data;
  };

public:
  struct PlotData1D
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
    bool log = false;                                       // logarithmic scale
    f64 aspectratio = 1;                                    // aspect ratio
    std::string save;                                       // save path
  };

  struct PlotData2D
  {
    cv::Mat z;
    f64 xmin = 0, xmax = 1, ymin = 0, ymax = 1;
    std::string xlabel = "x", ylabel = "y", zlabel = "z";
    bool interp = false;
    f64 aspectratio = 1;
    std::string cmap = "jet";
    std::string save;
  };

  struct PlotData3D
  {
    cv::Mat z;
    f64 xmin = 0, xmax = 1, ymin = 0, ymax = 1;
    std::string xlabel = "x", ylabel = "y", zlabel = "z";
    f64 aspectratio = 1;
    std::string cmap = "jet";
    u32 rstride = 0, cstride = 0;
    std::string save;
  };

  static void Initialize();
  static void Render();

  static void Plot(const std::string& name, const PlotData1D& data);
  static void Plot(const std::string& name, const PlotData2D& data);
  static void PlotSurf(const std::string& name, const PlotData3D& data);
  static void Plot(const std::string& name, const std::string& type, const py::dict& data);

  static void SavePlot(const std::string& path, const std::string& name, const PlotData1D& data);
  static void SavePlot(const std::string& path, const std::string& name, const PlotData2D& data);
  static void SavePlotSurf(const std::string& path, const std::string& name, const PlotData3D& data);
  static void SavePlot(const std::string& path, const std::string& name, const std::string& type, const py::dict& data);

private:
  inline static std::unordered_map<std::string, i32> mPlotIds;
  inline static i32 mId = 0;
  inline static std::queue<PlotData> mPlotQueue;
  inline static std::mutex mMutex;

  static void ScheldulePlot(const std::string& name, const std::string& type, const py::dict& data);
  static void PlotInternal(const PlotData& plotdata);

  static i32 GetPlotId(const std::string& name);
  static void AddDefaultScopeData(const std::string& name, py::dict& scope);
  static py::dict GetScopeData(const std::string& name, const py::dict& data);
  static py::dict GetScopeData(const std::string& name, const PlotData1D& data);
  static py::dict GetScopeData(const std::string& name, const PlotData2D& data);
  static py::dict GetScopeData(const std::string& name, const PlotData3D& data);
};
