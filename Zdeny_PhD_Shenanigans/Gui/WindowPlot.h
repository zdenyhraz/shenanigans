#pragma once
#include "ui_WindowPlot.h"

class WindowPlot : public QMainWindow
{
	Q_OBJECT

public:
	WindowPlot( std::function<void( WindowPlot * )> &OnClose );
	~WindowPlot();
	Ui::WindowPlot ui;
	std::shared_ptr<QCPColorMap> colorMap;
	std::shared_ptr<QCPColorScale> colorScale;
	std::shared_ptr<QCPMarginGroup> marginGroup;

private:
	std::function<void( WindowPlot * )> &OnClose;
	void closeEvent( QCloseEvent *event );

private slots:

};
