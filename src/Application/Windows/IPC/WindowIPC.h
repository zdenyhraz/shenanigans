#pragma once
#include "ui_WindowIPC.h"
#include "Core/globals.h"

class WindowIPC : public QMainWindow
{
  Q_OBJECT

public:
  WindowIPC(QWidget* parent, Globals* globals);

private:
  Ui::WindowIPC ui;
  Globals* globals;

private slots:
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
