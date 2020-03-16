#pragma once
#include "ui_WindowPlot.h"

class WindowPlot : public QMainWindow
{
	Q_OBJECT

public:
	WindowPlot( std::function<void( WindowPlot * )> &OnClose );
	~WindowPlot();
	Ui::WindowPlot ui;
	QCPColorMap *colorMap;
	QCPColorScale *colorScale;
	QCPMarginGroup *marginGroup;

private:
	std::function<void( WindowPlot * )> &OnClose;
	void closeEvent( QCloseEvent *event );

private slots:

};
