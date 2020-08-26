#include "stdafx.h"
#include "Plot1D.h"
#include "Plot2D.h"

std::function<void( std::string )> Plot1D::OnClose = []( std::string name )
{
	auto idx = Plot::plots.find( name );
	if ( idx != Plot::plots.end() )
	{
		delete idx->second;
		Plot::plots.erase( idx );
	}
};

void Plot1D::CloseAll()
{
	for ( auto &plt : Plot::plots )
	{
		delete plt.second;
		Plot::plots.erase( plt.first );
	}
}

void Plot1D::Reset( std::string name )
{
	auto idx = Plot::plots.find( name );
	if ( idx != Plot::plots.end() )
	{
		LOG_DEBUG( "Reseting 1Dplot '{}'", name );
		WindowPlot *windowPlot = idx->second;
		for ( int i = 0; i < windowPlot->ui.widget->graphCount(); i++ )
			windowPlot->ui.widget->graph( i )->data().data()->clear();

		windowPlot->ui.widget->rescaleAxes();
		windowPlot->ui.widget->replot();
		windowPlot->show();
	}
}

void Plot1D::plotcore( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel, std::string y1label, std::string y2label, std::vector<std::string> y1names, std::vector<std::string> y2names, std::vector<QPen> pens, std::string savepath )
{
	int y1cnt = y1s.size();
	int y2cnt = y2s.size();
	int ycnt = y1cnt + y2cnt;

	WindowPlot *windowPlot = RefreshGraph( name, ycnt, y1cnt, y2cnt, xlabel, y1label, y2label, y1names, y2names, pens );

	for ( int i = 0; i < ycnt; i++ )
	{
		if ( i < y1cnt )
			windowPlot->ui.widget->graph( i )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( y1s[i] ) );
		else
			windowPlot->ui.widget->graph( i )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( y2s[i - y1cnt] ) );
	}

	windowPlot->ui.widget->rescaleAxes();
	windowPlot->ui.widget->replot();
	windowPlot->show();

	if ( savepath != "" )
		windowPlot->ui.widget->savePng( QString::fromStdString( savepath ), 0, 0, 3, -1 );
}

void Plot1D::plotcore( double x, const std::vector<double> &y1s, const std::vector<double> &y2s, std::string name, std::string xlabel, std::string y1label, std::string y2label, std::vector<std::string> y1names, std::vector<std::string> y2names, std::vector<QPen> pens, std::string savepath )
{
	int y1cnt = y1s.size();
	int y2cnt = y2s.size();
	int ycnt = y1cnt + y2cnt;

	WindowPlot *windowPlot = RefreshGraph( name, ycnt, y1cnt, y2cnt, xlabel, y1label, y2label, y1names, y2names, pens );

	for ( int i = 0; i < ycnt; i++ )
	{
		if ( i < y1cnt )
			windowPlot->ui.widget->graph( i )->addData( x, y1s[i]  );
		else
			windowPlot->ui.widget->graph( i )->addData( x, y2s[i - y1cnt] );
	}

	windowPlot->ui.widget->rescaleAxes();
	windowPlot->ui.widget->replot();
	windowPlot->show();

	if ( savepath != "" )
		windowPlot->ui.widget->savePng( QString::fromStdString( savepath ), 0, 0, 3, -1 );
}

WindowPlot *Plot1D::RefreshGraph( std::string name, int ycnt, int y1cnt, int y2cnt, std::string xlabel, std::string y1label, std::string y2label, std::vector<std::string> &y1names, std::vector<std::string> &y2names, std::vector<QPen> pens )
{
	WindowPlot *windowPlot;
	auto idx = Plot::plots.find( name );
	if ( idx != Plot::plots.end() )
	{
		windowPlot = idx->second;
	}
	else
	{
		windowPlot = new WindowPlot( name, 1.2, OnClose );
		windowPlot->move( Plot::GetNewPlotPosition( windowPlot ) );
		Plot::plots[name] = windowPlot;
		SetupGraph( windowPlot, ycnt, y1cnt, y2cnt, xlabel, y1label, y2label, y1names, y2names, pens );
	}
	return windowPlot;
}

void Plot1D::SetupGraph( WindowPlot *windowPlot, int ycnt, int y1cnt, int y2cnt, std::string xlabel, std::string y1label, std::string y2label, std::vector<std::string> &y1names, std::vector<std::string> &y2names, std::vector<QPen> pens )
{
	windowPlot->ui.widget->xAxis->setTickLabelFont( Plot::fontTicks );
	windowPlot->ui.widget->yAxis->setTickLabelFont( Plot::fontTicks );
	windowPlot->ui.widget->yAxis2->setTickLabelFont( Plot::fontTicks );
	windowPlot->ui.widget->xAxis->setLabelFont( Plot::fontLabels );
	windowPlot->ui.widget->yAxis->setLabelFont( Plot::fontLabels );
	windowPlot->ui.widget->yAxis2->setLabelFont( Plot::fontLabels );
	windowPlot->ui.widget->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom );
	windowPlot->ui.widget->xAxis->setLabel( QString::fromStdString( xlabel ) );
	windowPlot->ui.widget->yAxis->setLabel( QString::fromStdString( y1label ) );
	windowPlot->ui.widget->yAxis2->setLabel( QString::fromStdString( y2label ) );
	//windowPlot->ui.widget->legend->setVisible( ycnt > 1 );
	windowPlot->ui.widget->axisRect()->insetLayout()->setInsetAlignment( 0, Qt::AlignBottom | Qt::AlignRight );
	for ( int i = 0; i < ycnt; i++ )
	{
		if ( i < y1cnt )
		{
			windowPlot->ui.widget->addGraph( windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis );
			if ( y1names.size() > i )
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( y1names[i] ) );
			else
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( "y1_" + to_string( i + 1 ) ) );
		}
		else
		{
			windowPlot->ui.widget->addGraph( windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis2 );
			if ( y2names.size() > i - y1cnt )
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( y2names[i - y1cnt] ) );
			else
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( "y2_" + to_string( i - y1cnt + 1 ) ) );
		}

		if ( i < pens.size() )
			windowPlot->ui.widget->graph( i )->setPen( pens[i] );
		else
			windowPlot->ui.widget->graph( i )->setPen( QPen( Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
	}

	if ( y2cnt > 0 )
		windowPlot->ui.widget->yAxis2->setVisible( true );

	windowPlot->show();
	QCoreApplication::processEvents();
}