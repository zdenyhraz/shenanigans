#pragma once
#include "ImGuiPlot.hpp"
#include "PyPlot.hpp"

using MainPlotter = ImGuiPlot;

template <typename T>
static void Plot(const std::string& name, T&& data)
{
  MainPlotter::Get().Plot(name, std::forward<T>(data));
}
