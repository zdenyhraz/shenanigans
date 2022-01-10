#pragma once
#include "ui_WindowFITS.h"
#include "Application/WindowData.h"

class WindowFITS : public QMainWindow
{
  Q_OBJECT

public:
  WindowFITS(QWidget* parent, WindowData* mWindowData);

private:
  Ui::WindowFITS ui;
  WindowData* mWindowData;

private slots:
  void fitsDownloader();
  void fitsDownloadChecker();
};
