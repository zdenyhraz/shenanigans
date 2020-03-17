#include "stdafx.h"
#include "Plot1D.h"

std::map<std::string, WindowPlot *> Plot1D::plots;

std::function<void( std::string )> Plot1D::OnClose = []( std::string name )
{
	auto idx = plots.find( name );
	if ( idx != plots.end() )
	{
		LOG_DEBUG( "Deleting 1Dplot '{}'", name );
		delete idx->second;
		plots.erase( idx );
	}
	else
	{
		LOG_ERROR( "1Dplot '{}' not found in plot registry (why tho?), not deleting", name );
	}
};

void Plot1D::CloseAll()
{
	for ( auto &plt : plots )
	{
		delete plt.second;
		plots.erase( plt.first );
	}
}

void Plot1D::plotinsides( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel, std::string y1label, std::string y2label, std::vector<std::string> &y1names, std::vector<std::string> &y2names, std::string savepath )
{
	WindowPlot *windowPlot;
	int y1cnt = y1s.size();
	int y2cnt = y2s.size();
	int ycnt = y1cnt + y2cnt;

	auto idx = plots.find( name );
	if ( idx != plots.end() )
	{
		LOG_DEBUG( "Updating 1Dplot '{}'", name );
		windowPlot = idx->second;
		windowPlot->Clear();
	}
	else
	{
		LOG_DEBUG( "Creating 1Dplot '{}'", name );
		windowPlot = new WindowPlot( name, OnClose );
	}
	plots[name] = windowPlot;

	windowPlot->ui.widget->xAxis->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->yAxis->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->yAxis2->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->xAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->yAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->yAxis2->setLabelFont( fontLabels );
	windowPlot->ui.widget->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom );
	windowPlot->ui.widget->xAxis->setLabel( QString::fromStdString( xlabel ) );
	windowPlot->ui.widget->yAxis->setLabel( QString::fromStdString( y1label ) );
	windowPlot->ui.widget->yAxis2->setLabel( QString::fromStdString( y2label ) );
	windowPlot->ui.widget->legend->setVisible( true );
	windowPlot->ui.widget->axisRect()->insetLayout()->setInsetAlignment( 0, Qt::AlignBottom | Qt::AlignRight );

	for ( int i = 0 ; i < ycnt; i++ )
	{
		if ( i < y1cnt )
		{
			windowPlot->ui.widget->addGraph( windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis );
			windowPlot->ui.widget->graph( i )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( y1s[i] ) );
			if ( y1names.size() > i )
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( y1names[i] ) );
			else
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( "y1_" + to_string( i + 1 ) ) );
		}
		else
		{
			windowPlot->ui.widget->addGraph( windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis2 );
			windowPlot->ui.widget->graph( i )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( y2s[i - y1cnt] ) );
			if ( y2names.size() > i - y1cnt )
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( y2names[i - y1cnt] ) );
			else
				windowPlot->ui.widget->graph( i )->setName( QString::fromStdString( "y2_" + to_string( i - y1cnt + 1 ) ) );
		}

		if ( plotPens1D.size() > i )
			windowPlot->ui.widget->graph( i )->setPen( plotPens1D[i] );
		else
			windowPlot->ui.widget->graph( i )->setPen( plotPens1D[1] );
	}

	if ( y2cnt > 0 )
		windowPlot->ui.widget->yAxis2->setVisible( true );

	windowPlot->ui.widget->rescaleAxes();
	windowPlot->ui.widget->replot();
	windowPlot->show();

	if ( savepath != "" )
	{
		windowPlot->ui.widget->savePng( QString::fromStdString( savepath ), 0, 0, 3, -1 );
	}
}

