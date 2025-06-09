#include "Gui/Application.hpp"
#include "Windows/IPCWindow.hpp"
#include "Windows/AstroWindow.hpp"
#include "Windows/RandomWindow.hpp"
#include "Windows/MicroserviceEditorWindow.hpp"
#include "NDA/Windows/ObjdetectColorWindow.hpp"
#include "NDA/Windows/ObjdetectObjectnessWindow.hpp"
#include "NDA/Windows/PipeTrackerWindow.hpp"

int main(int argc, char** argv)
try
{
  Application app("Shenanigans");
  app.SetIniPath("data/shenanigans/imgui.ini");
  app.SetFontPath("data/shenanigans/CascadiaCode.ttf");
  app.SetPlotWindowCount(2);
  app.AddWindow<PipeTrackerWindow>();
  app.AddWindow<ObjdetectObjectnessWindow>();
  app.AddWindow<ObjdetectColorWindow>();
  app.AddWindow<MicroserviceEditorWindow>();
  app.AddWindow<IPCWindow>();
  app.AddWindow<AstroWindow>();
  app.AddWindow<RandomWindow>();
  app.Run();
  return EXIT_SUCCESS;
}
catch (const ShenanigansException& e)
{
  e.Log();
  return EXIT_FAILURE;
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
  return EXIT_FAILURE;
}
