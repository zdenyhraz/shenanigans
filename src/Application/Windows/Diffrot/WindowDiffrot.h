#pragma once
#include "ui_WindowDiffrot.h"
#include "Core/globals.h"
#include "Astrophysics/diffrotResults.h"
#include "Astrophysics/diffrot.h"

class WindowDiffrot : public QMainWindow
{
  Q_OBJECT

public:
  WindowDiffrot(QWidget *parent, Globals *globals);

private:
  Ui::WindowDiffrot ui;
  Globals *globals;
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
