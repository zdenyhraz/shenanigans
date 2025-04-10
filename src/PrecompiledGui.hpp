#pragma once
#define ImDrawIdx unsigned int
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <implot.h>
#include <implot_internal.h>

#ifdef ENABLE_PROFILING
#  include <tracy/Tracy.hpp>
#  define PROFILE_FRAME FrameMark                           // use FrameMark for frames (at the end of each frame)
#  define PROFILE_FUNCTION ZoneScoped                       // use ZoneScoped once per scope (automatic name)
#  define PROFILE_SCOPE(name) ZoneNamedN(name, #name, true) // use ZoneNamedN for scopes inside ZoneScoped scope (user-supplied name), bool can turn it on/off
#  ifdef ENABLE_PROFILING_MEMORY
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
#  endif
#endif

namespace ImGui
{
inline bool SliderPercentage(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f%%", ImGuiSliderFlags flags = 0)
{
  float valuePercent = *v * 100;
  ImGui::SliderFloat(label, &valuePercent, v_min * 100, v_max * 100, format, flags);
  *v = valuePercent / 100;
  return true;
}
};
