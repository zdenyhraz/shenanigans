#include "ToolsWindow.hpp"
#include "UtilsCV/Vectmat.hpp"

void ToolsWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Tools"))
  {
    ImGui::Separator();
    ImGui::BulletText("Tools");

    ImGui::EndTabItem();
  }
}
