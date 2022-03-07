#pragma once
#include "ui_WindowFeatures.h"
#include "Application/Windows/WindowData.hpp"
#include "Features/FeatureMatch.hpp"

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
