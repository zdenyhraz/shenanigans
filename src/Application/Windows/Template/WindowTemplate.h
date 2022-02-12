#pragma once
#include "ui_WindowTemplate.h"
#include "Application/Windows/WindowData.h"

class WindowTemplate : public QMainWindow
{
  Q_OBJECT

public:
  WindowTemplate(QWidget* parent, WindowData* mWindowData);

private:
  Ui::WindowTemplate ui;
  WindowData* mWindowData;

private slots:
};
