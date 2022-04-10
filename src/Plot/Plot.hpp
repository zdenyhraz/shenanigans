#pragma once
#include "ImGuiPlot.hpp"
#include "PyPlot.hpp"

using MainPlotter = ImGuiPlot;

template <typename T>
static void Plot(const std::string& name, T&& data)
{
  MainPlotter::Get().Plot(name, std::forward<T>(data));
}

static void Plot1D(const std::string& name, PlotData1D&& data)
{
  MainPlotter::Get().Plot(name, std::move(data));
}

static void Plot2D(const std::string& name, PlotData2D&& data)
{
  MainPlotter::Get().Plot(name, std::move(data));
}
