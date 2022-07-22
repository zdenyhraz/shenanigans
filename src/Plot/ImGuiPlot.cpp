#include "ImGuiPlot.hpp"

ImGuiPlot::PlotData::PlotData(PlotData1D&& data1d)
{
  if (data1d.ylabels.size() < data1d.ys.size())
  {
    const auto origsize = data1d.ylabels.size();
    data1d.ylabels.resize(data1d.ys.size());
    for (usize i = origsize; i < data1d.ys.size(); ++i)
      data1d.ylabels[i] = fmt::format("y{}", i);
  }

  if (data1d.y2labels.size() < data1d.y2s.size())
  {
    const auto origsize = data1d.y2labels.size();
    data1d.y2labels.resize(data1d.y2s.size());
    for (usize i = origsize; i < data1d.y2s.size(); ++i)
      data1d.y2labels[i] = fmt::format("y2-{}", i);
  }

  data = std::move(data1d);
}

ImGuiPlot::PlotData::PlotData(PlotData2D&& data2d)
{
  const auto [zmin, zmax] = MinMax(data2d.z);
  data2d.zmin = zmin;
  data2d.zmax = zmax;
  data2d.z.convertTo(data2d.z, CV_32F);
  cv::resize(data2d.z, data2d.z, cv::Size(501, 501));
  data = std::move(data2d);
}

void ImGuiPlot::RenderPlot(const std::string& name, const PlotData& plotData) const
{
  if (ImGui::BeginTabItem(name.c_str() + 2))
  {
    if (std::holds_alternative<PlotData1D>(plotData.data))
      RenderPlot1D(name, std::get<PlotData1D>(plotData.data));
    else if (std::holds_alternative<PlotData2D>(plotData.data))
      RenderPlot2D(name, std::get<PlotData2D>(plotData.data));

    ImGui::EndTabItem();
  }
}

void ImGuiPlot::RenderPlot1D(const std::string& name, const PlotData1D& data) const
{
  if (ImPlot::BeginPlot(name.c_str(), ImVec2(-1, -1)))
  {
    ImPlot::GetStyle().Colormap = ImPlotColormap_Dark;
    ImPlot::SetupAxis(ImAxis_X1, data.xlabel.c_str(), ImPlotAxisFlags_None);
    if (data.log)
      ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

    ImPlot::SetupAxisLimits(ImAxis_X1, data.x.front(), data.x.back(), ImGuiCond_Always);
    if (not data.y2s.empty())
      ImPlot::SetupAxis(ImAxis_Y2, data.y2label.c_str(), ImPlotAxisFlags_AuxDefault);
    const auto x = data.x.data();
    for (usize i = 0; i < data.ys.size(); ++i)
    {
      const auto n = data.ys[i].size();
      if (n == 0)
        continue;

      const auto label = data.ylabels[i].c_str();
      const auto y = data.ys[i].data();
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      ImPlot::PlotLine(label, x, y, n);
    }
    for (usize i = 0; i < data.y2s.size(); ++i)
    {
      const auto n = data.y2s[i].size();
      if (n == 0)
        continue;

      const auto label = data.y2labels[i].c_str();
      const auto y = data.y2s[i].data();
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
      ImPlot::PlotLine(label, x, y, n);
    }
    ImPlot::EndPlot();
  }
}

void ImGuiPlot::RenderPlot2D(const std::string& name, const PlotData2D& data) const
{
  const auto& z = data.z;
  const f32 widthcb = 180;
  const f32 width = ImGui::GetContentRegionAvail().x - widthcb - ImGui::GetStyle().ItemSpacing.x;
  const f32 height = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.x;
  if (ImPlot::BeginPlot(name.c_str(), ImVec2(width, height)))
  {
    ImPlot::SetupAxes(data.xlabel.c_str(), data.ylabel.c_str());
    ImPlot::GetStyle().Colormap = data.cmap;
    ImPlot::PlotHeatmap(
        name.c_str(), std::bit_cast<f32*>(z.data), z.rows, z.cols, 0, 0, nullptr, ImVec2(data.xmin, data.ymin), ImVec2(data.xmax, data.ymax));
    ImPlot::EndPlot();
  }
  ImGui::SameLine();
  ImPlot::ColormapScale(name.c_str(), data.zmin, data.zmax, {widthcb, height}, nullptr, data.cmap);
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
      for (const auto& [name, data] : mPlots)
        RenderPlot(name, data);

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

  Plot(fmt::format("debug1d#{}", mPlots.size()), PlotData1D{.x = x, .ys = {y1, y2}, .ylabels = {"y1", "y2"}});
  LOG_DEBUG("Added one debug1d plot");

  Plot(fmt::format("debug2d#{}", mPlots.size()), PlotData2D{.z = Gaussian<f64>(n, n) + Random::Rand()});
  LOG_DEBUG("Added one debug2d plot");
}
