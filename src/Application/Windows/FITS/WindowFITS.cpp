
#include "WindowFITS.hpp"
#include "Astrophysics/FITS.hpp"

WindowFITS::WindowFITS(QWidget* parent, WindowData* windowData) : QMainWindow(parent), mWindowData(windowData)
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
