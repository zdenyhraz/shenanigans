
#include "WindowTemplate.h"

WindowTemplate::WindowTemplate(QWidget* parent, Globals* globals_) : QMainWindow(parent), globals(globals_)
{
  ui.setupUi(this);
}
