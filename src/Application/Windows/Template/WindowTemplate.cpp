
#include "WindowTemplate.hpp"

WindowTemplate::WindowTemplate(QWidget* parent, WindowData* windowData) : QMainWindow(parent), mWindowData(windowData)
{
  ui.setupUi(this);
}
