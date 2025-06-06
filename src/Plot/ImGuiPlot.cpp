#ifdef GLAPI

#  include "ImGuiPlot.hpp"

void ImGuiPlot::RenderInternal()
{
  PROFILE_FUNCTION;
  for (size_t plotLocation = 0; plotLocation < mWindowCount; ++plotLocation)
  {
    if (ImGui::Begin(mWindowCount > 1 ? fmt::format("Plot{}", static_cast<char>('A' + plotLocation)).c_str() : "Plot"))
    {
      if (ImGui::BeginTabBar("Plots", ImGuiTabBarFlags_AutoSelectNewTabs))
      {
        std::scoped_lock lock(mPlotsMutex);
        for (const auto& data : mPlots1D | std::ranges::views::filter([plotLocation](const auto& data) { return data.location == plotLocation; }))
          RenderInternal(data);
        for (const auto& data : mPlots2D | std::ranges::views::filter([plotLocation](const auto& data) { return data.location == plotLocation; }))
          RenderInternal(data);

        ImGui::EndTabBar();
      }
      ImGui::End();
    }
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
      for (size_t i = 0; i < data.ys.size(); ++i)
      {
        const auto n = data.ys[i].size();
        if (n == 0)
          continue;

        const auto y = data.ys[i].data();
        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        ImPlot::PlotLine(data.ylabels.size() > i ? data.ylabels[i].c_str() : fmt::format("y{}", i).c_str(), x, y, n);
      }
      for (size_t i = 0; i < data.y2s.size(); ++i)
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
    const float height = ImGui::GetContentRegionAvail().y;
    const float width = height * data.aspectratio;
    const float heightcb = height;
    const float widthcb = heightcb * 0.15;
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
        ImPlot::PlotHeatmap(data.name.c_str(), reinterpret_cast<float*>(data.z.data), data.z.rows, data.z.cols, 0, 0, nullptr, ImPlotPoint(data.xmin, data.ymin),
            ImPlotPoint(data.xmax, data.ymax));
      }

      if (data.z.channels() == 3)
      {
        if (data.image.texid == 0)
          data.image.Load(data.z); // has to run in main thread
        ImPlot::PlotImage(data.name.c_str(), static_cast<ImTextureID>(data.image.texid), ImPlotPoint(data.xmin, data.ymin), ImPlotPoint(data.xmax, data.ymax));
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
  if (cmap == "hot")
    return ImPlotColormap_Hot;
  if (cmap == "cool")
    return ImPlotColormap_Cool;

  return ImPlotColormap_Viridis;
}
#endif
