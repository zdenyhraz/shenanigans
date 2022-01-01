#pragma once
#include "ui_WindowPlot.h"

class WindowPlot : public QMainWindow
{
  Q_OBJECT

public:
  WindowPlot(std::string name, f64 colRowRatio, std::function<void(std::string)>& OnClose);
  ~WindowPlot();

  Ui::WindowPlot ui;
  QCPColorMap* colorMap;
  QCPColorScale* colorScale;
  QCPMarginGroup* marginGroup;

private:
  void closeEvent(QCloseEvent* event);
  std::string name;
  std::function<void(std::string)>& OnClose;

private slots:
};
