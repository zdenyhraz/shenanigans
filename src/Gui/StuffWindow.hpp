#pragma once
#include "Window.hpp"

class StuffWindow : public Window
{
  void EvolutionOptimization(bool meta);
  void PlotTest();
  void UnevenIlluminationCLAHE();
  void UnevenIlluminationHomomorphic();

  bool showImGuiDemoWindow = false;
  bool showImPlotDemoWindow = false;

public:
  void Render() override;
};
