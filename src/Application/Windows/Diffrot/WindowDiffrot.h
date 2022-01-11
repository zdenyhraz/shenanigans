#pragma once
#include "ui_WindowDiffrot.h"
#include "Application/WindowData.h"
#include "Astrophysics/Diffrotresults.h"
#include "Astrophysics/Diffrot.h"

class WindowDiffrot : public QMainWindow
{
  Q_OBJECT

public:
  WindowDiffrot(QWidget* parent, WindowData* mWindowData);

private:
  Ui::WindowDiffrot ui;
  WindowData* mWindowData;
  DiffrotResults drres;
  DiffrotSettings drset;

private slots:
  void calculateDiffrot();
  void showResults();
  void showIPC();
  void checkDiskShifts();
  void saveDiffrot();
  void loadDiffrot();
  void optimizeDiffrot();
  void UpdateDrset();
  void video();
  void movingPeak();
  FitsTime GetStartFitsTime();
};
