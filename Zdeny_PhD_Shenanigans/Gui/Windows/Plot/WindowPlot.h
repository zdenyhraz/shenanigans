#pragma once
#include "ui_WindowPlot.h"

class WindowPlot : public QMainWindow
{
	Q_OBJECT

public:
	WindowPlot( std::string name, double colRowRatio, std::function<void( std::string )> &OnClose );
	~WindowPlot();

	Ui::WindowPlot ui;
	QCPColorMap *colorMap;
	QCPColorScale *colorScale;
	QCPMarginGroup *marginGroup;

private:
	std::function<void( std::string )> &OnClose;
	void closeEvent( QCloseEvent *event );
	std::string name;

private slots:

};
