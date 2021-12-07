
#include "WindowTemplate.h"

WindowTemplate::WindowTemplate(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
	ui.setupUi(this);
}

