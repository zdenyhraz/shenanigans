#pragma once
#include "ui_WindowIPC.h"
#include "Application/Windows/WindowData.hpp"

class WindowIPC : public QMainWindow
{
  Q_OBJECT

public:
  WindowIPC(QWidget* parent, WindowData* mWindowData);

private:
  Ui::WindowIPC ui;
  WindowData* mWindowData;

public slots:
  void RefreshIPCparameters();
  void RefreshIPCparametersAndExit();
  void Optimize();
  void PlotObjectiveFunctionLandscape();
  void PlotUpsampleCoefficientAccuracyDependence();
  void PlotNoiseAccuracyDependence();
  void PlotNoiseOptimalBPHDependence();
  void PlotImageSizeAccuracyDependence();
  void align();
  void alignXY();
  void CalculateFlow();
  void features();
  void linearFlow();
  void constantFlow();
  void ShowDebugStuff();
};
