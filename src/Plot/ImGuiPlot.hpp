#pragma once

class ImGuiPlot
{
  struct PlotData
  {
    std::string name;
  };

  inline static std::vector<PlotData> mPlots;

public:
  static void Render();
};
