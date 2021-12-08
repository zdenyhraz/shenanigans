
#include "Core/functionsBaseSTL.h"
#include "WindowPlot.h"

WindowPlot::WindowPlot(std::string name_, double colRowRatio, std::function<void(std::string)>& OnClose_) : QMainWindow(), name(name_), OnClose(OnClose_)
{
  ui.setupUi(this);
  int wRows = 400;
  int wCols = colRowRatio * wRows + 100;
  ui.widget->setFixedSize(clamp(wCols, 0, QApplication::desktop()->width()), clamp(wRows, 0, QApplication::desktop()->height()));
  setFixedSize(ui.widget->width(), ui.widget->height() + 20);
  setWindowTitle(QString::fromStdString(name));
}

WindowPlot::~WindowPlot()
{
}

void WindowPlot::closeEvent(QCloseEvent* event)
{
  OnClose(name);
}
