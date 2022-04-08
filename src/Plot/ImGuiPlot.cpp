#include "ImGuiPlot.hpp"

void ImGuiPlot::Render()
{
  static constexpr usize n = 1000;
  float x_data[n];
  float y_data[n];

  ImGui::Begin("Plots");
  if (ImPlot::BeginPlot("My Plot 1"))
  {
    ImPlot::PlotLine("My Line Plot 1", x_data, y_data, n);
    ImPlot::EndPlot();
  }
  if (ImPlot::BeginPlot("My Plot 2"))
  {
    ImPlot::PlotLine("My Line Plot 2", x_data, y_data, n);
    ImPlot::EndPlot();
  }
  ImGui::End();
}
