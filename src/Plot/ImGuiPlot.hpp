#pragma once

class ImGuiPlot
{
  struct PlotData
  {
    std::string name;
  };

  inline static std::vector<PlotData> mPlots;

public:
  static void Render()
  {
    float x_data[6] = {0, 1, 2, 3, 4, 5};
    float y_data[6] = {7, 5, 6, 1, 9, 8};

    ImGui::Begin("My Window");
    ImPlot::BeginPlot("My Plot");
    ImPlot::PlotLine("My Line Plot", x_data, y_data, 6);
    ImPlot::EndPlot();
    ImGui::End();

    for (const auto& plot : mPlots)
    {
    }
  }
};
