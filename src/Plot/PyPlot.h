#pragma once

class Plt
{
public:
  struct Data1D
  {
    std::vector<f64> x, y, y2;                              // plot data
    std::vector<std::vector<f64>> ys, y2s;                  // plot multi data
    std::string xlabel = "x", ylabel = "y", y2label = "y2"; // plot labels
    std::string label_y = "y", label_y2 = "y2";             // data labels
    std::vector<std::string> label_ys, label_y2s;           // multi data labels
    std::string linestyle_y, linestyle_y2;                  // line styles
    std::vector<std::string> linestyle_ys, linestyle_y2s;   // multi line styles
    std::string title;                                      // plot title
  };

  struct Data2D
  {
    cv::Mat z;
    std::string xlabel = "x", ylabel = "y", zlabel = "z";
    std::string title;
  };

  static void Plot(std::string&& name, Data1D&& data);
  static void Plot(std::string&& name, Data2D&& data);

private:
  inline static std::unordered_map<std::string, u32> mPlotIds;
  inline static u32 mId = 0;

  static void CheckIfPlotExists(const std::string& name);
  static py::dict GetScopeData(const std::string& name, const Data1D& data);
  static py::dict GetScopeData(const std::string& name, const Data2D& data);
};