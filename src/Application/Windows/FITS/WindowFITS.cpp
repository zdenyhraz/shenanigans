
#include "WindowFITS.h"

WindowFITS::WindowFITS(QWidget* parent, Globals* globals_) : QMainWindow(parent), globals(globals_)
{
  ui.setupUi(this);

  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(fitsDownloader()));
  connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(fitsDownloadChecker()));
}

void WindowFITS::fitsDownloader()
{
  fitsDownloaderImpl();
}

void WindowFITS::fitsDownloadChecker()
{
  fitsDownloadCheckerImpl();
}
