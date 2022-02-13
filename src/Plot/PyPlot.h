#pragma once

class Plt
{
public:
  struct Data
  {
    std::vector<f64> x, y, y2;                            // plot data
    std::vector<std::vector<f64>> ys, y2s;                // plot multi data
    std::string xlabel, ylabel, y2label, zlabel;          // plot labels
    std::string label_y, label_y2;                        // data labels
    std::vector<std::string> label_ys, label_y2s;         // multi data labels
    std::string linestyle_y, linestyle_y2;                // line styles
    std::vector<std::string> linestyle_ys, linestyle_y2s; // multi line styles
    std::string title;                                    // plot title
  };

  static void Plot(std::string&& name, Data&& data);

private:
  inline static std::unordered_map<std::string, u32> mPlotIds;
  inline static u32 mId = 0;

  static py::dict GetScopeData(const std::string& name, const Data& data);
};