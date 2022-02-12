#pragma once
#include "ui_WindowFiltering.h"
#include "Application/Windows/WindowData.h"

class WindowFiltering : public QMainWindow
{
  Q_OBJECT

public:
  WindowFiltering(QWidget* parent, WindowData* mWindowData);

private:
  Ui::WindowFiltering ui;
  WindowData* mWindowData;

private slots:
  void HistogramEqualize();
};
