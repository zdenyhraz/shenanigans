#include "ImGuiPlot.hpp"

void ImGuiPlot::RenderInternal()
{
  PROFILE_FUNCTION;
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
      for (const auto& data : mPlots1D)
        RenderInternal(data);
      for (const auto& data : mPlots2D)
        RenderInternal(data);

      ImGui::EndTabBar();
    }
    ImGui::End();
  }
}

void ImGuiPlot::RenderInternal(const PlotData1D& data)
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem(data.name.c_str()))
  {
    if (ImPlot::BeginPlot(data.name.c_str(), ImVec2(-1, -1)))
    {
      ImPlot::GetStyle().Colormap = ImPlotColormap_Dark;
      ImPlot::SetupAxis(ImAxis_X1, data.xlabel.c_str(), ImPlotAxisFlags_None);
      if (data.log)
        ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

      if (not data.x.empty())
        ImPlot::SetupAxisLimits(ImAxis_X1, data.x.front(), data.x.back(), ImGuiCond_Always);
      else
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, 1, ImGuiCond_Always);

      if (not data.y2s.empty())
        ImPlot::SetupAxis(ImAxis_Y2, data.y2label.c_str(), ImPlotAxisFlags_AuxDefault);
      const auto x = data.x.data();
      for (usize i = 0; i < data.ys.size(); ++i)
      {
        const auto n = data.ys[i].size();
        if (n == 0)
          continue;

        const auto y = data.ys[i].data();
        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        ImPlot::PlotLine(data.ylabels.size() > i ? data.ylabels[i].c_str() : fmt::format("y{}", i).c_str(), x, y, n);
      }
      for (usize i = 0; i < data.y2s.size(); ++i)
      {
        const auto n = data.y2s[i].size();
        if (n == 0)
          continue;

        const auto y = data.y2s[i].data();
        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
        ImPlot::PlotLine(data.y2labels.size() > i ? data.y2labels[i].c_str() : fmt::format("y{}", i).c_str(), x, y, n);
      }
      ImPlot::EndPlot();
    }
    ImGui::EndTabItem();
  }
}

void ImGuiPlot::RenderInternal(const PlotData2D& data)
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem(data.name.c_str()))
  {
    const f32 height = ImGui::GetContentRegionAvail().y;
    const f32 width = height / data.z.rows * data.z.cols * 1.1;
    const f32 widthcb = height * 0.175f;
    if (ImPlot::BeginPlot(data.name.c_str(), ImVec2(width, height)))
    {
      ImPlot::SetupAxes(data.xlabel.c_str(), data.ylabel.c_str());
      ImPlot::GetStyle().Colormap = GetColormap(data.cmap);
      ImPlot::PlotHeatmap(data.name.c_str(), std::bit_cast<f32*>(data.z.data), data.z.rows, data.z.cols, 0, 0, nullptr, ImVec2(data.xmin, data.ymin), ImVec2(data.xmax, data.ymax));
      ImPlot::EndPlot();
    }
    ImGui::SameLine();
    if (data.colorbar)
      ImPlot::ColormapScale(data.name.c_str(), data.zmin, data.zmax, {widthcb, height * 0.912f});
    ImGui::EndTabItem();
  }
}

ImPlotColormap ImGuiPlot::GetColormap(const std::string& cmap)
{
  if (cmap == "jet")
    return ImPlotColormap_Jet;
  if (cmap == "gray")
    return ImPlotColormap_Greys;

  return ImPlotColormap_Jet;
}

void ImGuiPlot::Debug()
{
  static constexpr usize n = 101;
  std::vector<f64> x(n);
  std::vector<f64> y1(n);
  std::vector<f64> y2(n);

  for (usize i = 0; i < n; ++i)
  {
    x[i] = static_cast<f64>(i) / (n - 1);
    y1[i] = std::sin(2 * std::numbers::pi * x[i]);
    y2[i] = std::cos(2 * std::numbers::pi * x[i]);
  }

  Plot({.name = fmt::format("debug1d#{}", mPlots1D.size()), .x = x, .ys = {y1, y2}, .ylabels = {"y1", "y2"}});
  LOG_DEBUG("Added one debug1d plot");

  Plot({.name = fmt::format("debug2d#{}", mPlots2D.size()), .z = Gaussian<f32>(n, n) + Random::Rand<f32>()});
  LOG_DEBUG("Added one debug2d plot");
}
