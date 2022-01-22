#pragma once
#include "ui_WindowDiffrot.h"
#include "Application/WindowData.h"
#include "Astrophysics/DifferentialRotation.h"

class WindowDiffrot : public QMainWindow
{
  Q_OBJECT

public:
  WindowDiffrot(QWidget* parent, WindowData* mWindowData);

private:
  Ui::WindowDiffrot ui;
  WindowData* mWindowData;
  DifferentialRotation GetDifferentialRotation(i32 xsizeoverride = 0);
  std::string GetDataPath();
  i32 GetIdstart();

private slots:
  void Calculate();
  void Load();
  void Optimize();
  void PlotMeridianCurve();
};
