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
      static const ImPlotAxisFlags axesFlags = ImPlotAxisFlags_AutoFit;
      ImPlot::SetupAxis(ImAxis_X1, data.xlabel.c_str(), axesFlags);
      if (not data.ys.empty())
        ImPlot::SetupAxis(ImAxis_Y1, data.ylabel.c_str(), axesFlags);
      if (not data.y2s.empty())
        ImPlot::SetupAxis(ImAxis_Y2, data.y2label.c_str(), axesFlags);
      if (data.log)
        ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

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
    const f32 width = height * data.aspectratio;
    const f32 heightcb = height;
    const f32 widthcb = heightcb * 0.15;
    static const ImPlotFlags plotFlags = ImPlotFlags_NoLegend; // | ImPlotFlags_Crosshairs;
    static const ImPlotAxisFlags axesFlagsX = ImPlotAxisFlags_None;
    const ImPlotAxisFlags axesFlagsY = data.ymin > data.ymax ? ImPlotAxisFlags_Invert : ImPlotAxisFlags_None;
    if (ImPlot::BeginPlot(data.name.c_str(), ImVec2(width, height), plotFlags))
    {
      ImPlot::SetupAxes(data.xlabel.c_str(), data.ylabel.c_str(), axesFlagsX, axesFlagsY);
      ImPlot::SetupAxisLimits(ImAxis_X1, data.xmin, data.xmax);
      ImPlot::SetupAxisLimits(ImAxis_Y1, std::min(data.ymin, data.ymax), std::max(data.ymax, data.ymin));
      ImPlot::GetStyle().Colormap = GetColormap(data.cmap);

      if (data.z.channels() == 1)
      {
        ImPlot::PlotHeatmap(
            data.name.c_str(), std::bit_cast<f32*>(data.z.data), data.z.rows, data.z.cols, 0, 0, nullptr, ImPlotPoint(data.xmin, data.ymin), ImPlotPoint(data.xmax, data.ymax));
      }

      if (data.z.channels() == 3)
      {
        if (data.image.texid == 0)
          data.image.Load(data.z); // has to run in main thread
        ImPlot::PlotImage(
            data.name.c_str(), std::bit_cast<ImTextureID>(static_cast<intptr_t>(data.image.texid)), ImPlotPoint(data.xmin, data.ymin), ImPlotPoint(data.xmax, data.ymax));
      }

      ImPlot::EndPlot();
    }
    ImGui::SameLine();
    if (data.colorbar)
      ImPlot::ColormapScale("##NoLabel", data.zmin, data.zmax, ImVec2{widthcb, heightcb}, "%g", ImPlotColormapScaleFlags_NoLabel);

    ImGui::EndTabItem();
  }
}

ImPlotColormap ImGuiPlot::GetColormap(const std::string& cmap)
{
  if (cmap == "gray")
    return ImPlotColormap_Viridis;
  if (cmap == "jet")
    return ImPlotColormap_Jet;
  if (cmap == "viridis")
    return ImPlotColormap_Viridis;

  return ImPlotColormap_Viridis;
}

void ImGuiPlot::Debug()
{
  static constexpr usize n = 501;
  const f64 y1A = Random::Rand<f64>(0.5, 1);
  const f64 y2A = Random::Rand<f64>(0.5, 1);
  const f64 y1F = Random::Rand<f64>(2, 5);
  const f64 y2F = Random::Rand<f64>(2, 5);
  std::vector<f64> x(n);
  std::vector<f64> y1(n);
  std::vector<f64> y2(n);
  for (usize i = 0; i < n; ++i)
  {
    x[i] = static_cast<f64>(3 * i) / (n - 1);
    y1[i] = y1A * std::sin(y1F * std::numbers::pi * x[i]);
    y2[i] = y2A * std::cos(y2F * std::numbers::pi * x[i]);
  }

  ImGuiPlot::Plot({.name = fmt::format("debug1d({})", mPlots1D.size()), .x = x, .ys = {y1, y2}, .ylabels = {"Asin(fx)", "Acos(fx)"}});
  LOG_DEBUG("Added one debug1d plot");

  Plot({.name = fmt::format("debug2d({})", mPlots2D.size()),
      .z = Random::Rand<f32>(0, 1) * Gaussian<f32>(n, Random::Rand<f32>(0, 0.5) * n),
      .xmin = -3,
      .xmax = 3,
      .ymin = -5,
      .ymax = 5,
      .xlabel = "xlabeeel",
      .ylabel = "ylabeeel",
      .zlabel = "zlabeeel"});
}
