#pragma once
#include "ImGuiPlot.hpp"
#include "PyPlot.hpp"
#include "Utils/Service.hpp"

namespace Plot
{
template <typename T>
static void Plot(const std::string& name, T&& data)
{
  Service<ImGuiPlot>::Get().Plot(name, std::forward<T>(data));
}

static void Render()
{
  Service<ImGuiPlot>::Get().Render();
}

}
