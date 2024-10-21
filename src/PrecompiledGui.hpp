#pragma once
#define ImDrawIdx unsigned int
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <implot.h>
#include <implot_internal.h>

#ifdef ENABLE_PROFILING
  #include <Tracy.hpp>
  // use FrameMark for frames (at the end of each frame)
  // use ZoneScoped once per scope (automatic name)
  // use ZoneScopedN once per scope (user-supplied name)
  // use ZoneNamedN for scopes inside ZoneScoped scope (user-supplied name)
  // bool parameter of ZoneNamed can turn it on/off
  #define PROFILE_FRAME FrameMark
  #define PROFILE_FUNCTION ZoneScoped
  #define PROFILE_SCOPE(name) ZoneNamedN(name, #name, true)

inline void* operator new(std::size_t count)
{
  auto ptr = malloc(count);
  TracyAlloc(ptr, count);
  return ptr;
}

inline void operator delete(void* ptr) noexcept
{
  TracyFree(ptr);
  free(ptr);
}
#endif

namespace ImGui
{
inline bool SliderPercentage(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f%%", ImGuiSliderFlags flags = 0)
{
  float valuePercent = *v * 100;
  ImGui::SliderFloat(label, &valuePercent, v_min * 100, v_max * 100, format, flags);
  *v = valuePercent / 100;
  return true;
}
};
