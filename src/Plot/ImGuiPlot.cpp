#include "ImGuiPlot.hpp"

void ImGuiPlot::Render()
{
  static constexpr usize n = 1000;
  f32 x[n];
  f32 y1[n];
  f32 y2[n];

  for (usize i = 0; i < n; ++i)
  {
    x[i] = static_cast<f32>(i) / (n - 1);
    y1[i] = std::sin(2 * std::numbers::pi * x[i]);
    y2[i] = std::cos(2 * std::numbers::pi * x[i]);
  }

  if (ImGui::Begin("Plot"))
  {
    if (ImGui::BeginTabBar("Plots"))
    {
      if (ImGui::BeginTabItem("Plot 1 tab"))
      {
        if (ImPlot::BeginPlot("Plot 1", ImVec2(-1, -1)))
        {
          ImPlot::PlotLine("y1", x, y1, n);
          ImPlot::PlotLine("y2", x, y2, n);
          ImPlot::EndPlot();
        }
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Plot 2 tab"))
      {
        if (ImPlot::BeginPlot("Plot 2", ImVec2(-1, -1)))
        {
          ImPlot::PlotLine("y1", x, y2, n);
          ImPlot::PlotLine("y2", x, y1, n);
          ImPlot::EndPlot();
        }
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::End();
  }
}
