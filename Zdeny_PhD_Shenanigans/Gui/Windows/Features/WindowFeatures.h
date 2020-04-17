#pragma once
#include "ui_WindowFeatures.h"
#include "Core/globals.h"

class WindowFeatures : public QMainWindow
{
	Q_OBJECT

public:
	WindowFeatures( QWidget *parent, Globals *globals );

private:
	Ui::WindowFeatures ui;
	Globals *globals;

private slots:

};
