#include "stdafx.h"
#include "WindowFITS.h"

WindowFITS::WindowFITS(QWidget *parent, Globals *globals) : QMainWindow(parent), globals(globals)
{
  ui.setupUi(this);

  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(fitsDownloader()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(fitsDownloadChecker()));
}

void WindowFITS::fitsDownloader() { fitsDownloaderImpl(); }

void WindowFITS::fitsDownloadChecker() { fitsDownloadCheckerImpl(); }
