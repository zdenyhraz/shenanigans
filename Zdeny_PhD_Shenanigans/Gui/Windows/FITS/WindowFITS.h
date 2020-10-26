#pragma once
#include "ui_WindowFITS.h"
#include "Core/globals.h"

class WindowFITS : public QMainWindow
{
  Q_OBJECT

public:
  WindowFITS(QWidget *parent, Globals *globals);

private:
  Ui::WindowFITS ui;
  Globals *globals;

private slots:
  void fitsDownloader();
  void fitsDownloadChecker();
};
