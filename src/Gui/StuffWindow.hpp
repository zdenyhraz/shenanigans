#pragma once

class StuffWindow
{
public:
  static void Initialize();
  static void Render();

private:
  inline static bool showImGuiDemoWindow = false;
  inline static bool showImPlotDemoWindow = false;

  static void EvolutionOptimization(bool meta);
  static void FalseCorrelationsRemoval();
  static void PlotTest();
  static void UnevenIlluminationCLAHE();
  static void UnevenIlluminationHomomorphic();
};
