#include "ImGuiPlot.hpp"

void ImGuiPlot::RenderPlot(const PlotData& plotData)
{
  if (ImGui::BeginTabItem(plotData.name.c_str()))
  {
    if (ImPlot::BeginPlot(plotData.name.c_str(), ImVec2(-1, -1)))
    {
      if (std::holds_alternative<PlotData1D>(plotData.data))
        RenderPlot1D(plotData.name, std::get<PlotData1D>(plotData.data));
      else if (std::holds_alternative<PlotData2D>(plotData.data))
        RenderPlot2D(plotData.name, std::get<PlotData2D>(plotData.data));

      ImPlot::EndPlot();
    }
    ImGui::EndTabItem();
  }
}

void ImGuiPlot::RenderPlot1D(const std::string&, const PlotData1D& data)
{
  for (usize i = 0; i < data.ys.size(); ++i)
  {
    const auto ylabelAuto = fmt::format("y{}", i);
    const auto ylabel = i < data.ylabels.size() ? data.ylabels[i].c_str() : ylabelAuto.c_str();
    const auto x = data.x.data();
    const auto y = data.ys[i].data();
    const auto n = data.ys[i].size();
    ImPlot::PlotLine(ylabel, x, y, n);
  }
}

void ImGuiPlot::RenderPlot2D(const std::string& name, const PlotData2D& data)
{
  const auto& z = data.z;
  const auto height = z.rows;
  const auto width = z.cols;
  const auto xmin = data.xmin;
  const auto ymin = data.ymin;
  const auto xmax = data.xmax;
  const auto ymax = data.ymax;

  ImPlot::PlotHeatmap(name.c_str(), std::bit_cast<f64*>(z.data), height, width, 0., 1, NULL, ImVec2(xmin, ymin), ImVec2(xmax, ymax));
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
  static constexpr usize n = 1000;
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
  static constexpr usize n = 1000;
  cv::Mat z(n, n, CV_64F);
  for (i32 r = 0; r < z.rows; ++r)
  {
    auto zp = z.ptr<f64>(r);
    for (i32 c = 0; c < z.cols; ++c)
      zp[c] = static_cast<f64>(r + c) / (z.rows + z.cols);
  }

  Plot(fmt::format("debug2d#{}", debugindex++), PlotData2D{.z = z});
  LOG_DEBUG("Added one debug2d plot");
}
