#pragma once
#include "ui_WindowFiltering.h"
#include "Core/globals.h"

class WindowFiltering : public QMainWindow
{
  Q_OBJECT

public:
  WindowFiltering(QWidget *parent, Globals *globals);

private:
  Ui::WindowFiltering ui;
  Globals *globals;

private slots:
  void HistogramEqualize();
};
