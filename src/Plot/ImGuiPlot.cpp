#include "ImGuiPlot.hpp"

void ImGuiPlot::RenderPlot(const PlotData& plotData)
{
  if (ImGui::BeginTabItem(plotData.name.c_str() + 2))
  {
    if (std::holds_alternative<PlotData1D>(plotData.data))
      RenderPlot1D(plotData.name, std::get<PlotData1D>(plotData.data));
    else if (std::holds_alternative<PlotData2D>(plotData.data))
      RenderPlot2D(plotData.name, std::get<PlotData2D>(plotData.data));

    ImGui::EndTabItem();
  }
}

void ImGuiPlot::RenderPlot1D(const std::string& name, const PlotData1D& data)
{
  if (ImPlot::BeginPlot(name.c_str(), ImVec2(-1, -1)))
  {
    for (usize i = 0; i < data.ys.size(); ++i)
    {
      const auto ylabelAuto = fmt::format("y{}", i);
      const auto ylabel = i < data.ylabels.size() ? data.ylabels[i].c_str() : ylabelAuto.c_str();
      const auto x = data.x.data();
      const auto y = data.ys[i].data();
      const auto n = data.ys[i].size();

      ImPlot::GetStyle().Colormap = ImPlotColormap_Dark;
      ImPlot::PlotLine(ylabel, x, y, n);
    }
    ImPlot::EndPlot();
  }
}

void ImGuiPlot::RenderPlot2D(const std::string& name, const PlotData2D& data)
{
  const auto& z = data.z;
  const f32 widthcb = 180;
  const float width = ImGui::GetContentRegionAvail().x - widthcb - ImGui::GetStyle().ItemSpacing.x;
  const float height = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.x;
  if (ImPlot::BeginPlot(name.c_str(), ImVec2(width, height)))
  {
    ImPlot::GetStyle().Colormap = ImPlotColormap_Jet;
    ImPlot::PlotHeatmap(name.c_str(), std::bit_cast<f64*>(z.data), z.rows, z.cols, 0, 0, nullptr, ImVec2(data.xmin, data.ymin), ImVec2(data.xmax, data.ymax));
    ImPlot::EndPlot();
  }
  ImGui::SameLine();
  ImPlot::ColormapScale(name.c_str(), 0, 1, {widthcb, height}, ImPlotColormap_Jet);
}

void ImGuiPlot::Render()
{
  if (ImGui::Begin("Plot"))
  {
    if (ImGui::Button("Clear"))
      Clear();
    ImGui::SameLine();
    if (ImGui::Button("Debug"))
      Debug();

    if (ImGui::BeginTabBar("Plots"))
    {
      std::scoped_lock lock(mPlotsMutex);
      for (const auto& [name, plot] : mPlots)
        RenderPlot(plot);

      ImGui::EndTabBar();
    }

    ImGui::End();
  }
}

void ImGuiPlot::Clear()
{
  std::scoped_lock lock(mPlotsMutex);
  const auto n = mPlots.size();
  mPlots.clear();
  if (n > 0)
    LOG_DEBUG("Cleared {} plots", n);
}

void ImGuiPlot::Debug()
{
  Debug1D();
  Debug2D();
}

void ImGuiPlot::Debug1D()
{
  static usize debugindex = 0;
  static constexpr usize n = 1001;
  std::vector<f64> x(n);
  std::vector<f64> y1(n);
  std::vector<f64> y2(n);

  for (usize i = 0; i < n; ++i)
  {
    x[i] = static_cast<f64>(i) / (n - 1);
    y1[i] = std::sin(2 * std::numbers::pi * x[i]);
    y2[i] = std::cos(2 * std::numbers::pi * x[i]);
  }

  Plot(fmt::format("debug1d#{}", debugindex++), PlotData1D{.x = x, .ys = {y1, y2}, .ylabels = {"y1", "y2"}});
  LOG_DEBUG("Added one debug1d plot");
}

void ImGuiPlot::Debug2D()
{
  static usize debugindex = 0;
  static constexpr usize n = 501;

  Plot(fmt::format("debug2d#{}", debugindex++), PlotData2D{.z = Gaussian<f64>(n, n)});
  LOG_DEBUG("Added one debug2d plot");
}
