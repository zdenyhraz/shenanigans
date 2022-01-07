
#include "Utils/functionsBaseSTL.h"
#include "WindowPlot.h"

WindowPlot::WindowPlot(const std::string& name_, f64 colRowRatio, std::function<void(std::string)>& OnClose_) : QMainWindow(), name(name_), OnClose(OnClose_)
{
  ui.setupUi(this);
  i32 wRows = 400;
  i32 wCols = colRowRatio * wRows + 100;
  ui.widget->setFixedSize(clamp(wCols, 0, QApplication::desktop()->width()), clamp(wRows, 0, QApplication::desktop()->height()));
  setFixedSize(ui.widget->width(), ui.widget->height() + 20);
  setWindowTitle(QString::fromStdString(name));
}

void WindowPlot::closeEvent(QCloseEvent* event)
{
  OnClose(name);
}
