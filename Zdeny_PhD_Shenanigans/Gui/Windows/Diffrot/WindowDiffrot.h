#pragma once
#include "ui_WindowDiffrot.h"
#include "Core/globals.h"
#include "Astrophysics/diffrotResults.h"

class WindowDiffrot : public QMainWindow
{
	Q_OBJECT

public:
	WindowDiffrot( QWidget *parent, Globals *globals );

private:
	Ui::WindowDiffrot ui;
	Globals *globals;
	DiffrotResults *diffrotResults;

private slots:
	void calculateDiffrot();
	void showResults();
	void showIPC();
	void checkDiskShifts();
};
