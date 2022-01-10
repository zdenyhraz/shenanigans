#pragma once
#include "ui_WindowFeatures.h"
#include "Application/WindowData.h"
#include "Features/FeatureMatch.h"

class WindowFeatures : public QMainWindow
{
  Q_OBJECT

public:
  WindowFeatures(QWidget* parent, WindowData* mWindowData);

private:
  Ui::WindowFeatures ui;
  WindowData* mWindowData;

private slots:
  void FeatureMatch();
};
