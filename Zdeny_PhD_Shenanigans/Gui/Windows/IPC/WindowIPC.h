#pragma once
#include "ui_WindowIPC.h"
#include "Core/globals.h"

class WindowIPC : public QMainWindow
{
  Q_OBJECT

public:
  WindowIPC(QWidget *parent, Globals *globals);

private:
  Ui::WindowIPC ui;
  Globals *globals;

private slots:
  void RefreshIPCparameters(bool init = false);
  void RefreshIPCparametersAndExit();
  void Optimize();
  void align();
  void alignXY();
  void flowMap();
  void features();
  void linearFlow();
  void constantFlow();
};
