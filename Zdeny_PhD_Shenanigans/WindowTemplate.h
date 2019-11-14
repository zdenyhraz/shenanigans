#pragma once
#include "ui_WindowTemplate.h"
#include "globals.h"

class WindowTemplate : public QMainWindow
{
	Q_OBJECT

public:
	WindowTemplate(QWidget* parent, Globals* globals);

private:
	Ui::WindowTemplate ui;
	Globals* globals;

private slots:

};
