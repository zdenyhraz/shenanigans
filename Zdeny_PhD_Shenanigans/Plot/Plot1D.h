#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "IPlot1D.h"
#include "Gui/WindowPlot.h"
#include "Plot.h"

struct Plot1D : IPlot1D
{
	QCustomPlot *widget;

	inline Plot1D( QCustomPlot *widget ) : widget( widget )
	{
		widget->clearPlottables();
		widget->addGraph();
		widget->xAxis->setTickLabelFont( fontTicks );
		widget->yAxis->setTickLabelFont( fontTicks );
		widget->xAxis->setLabelFont( fontLabels );
		widget->yAxis->setLabelFont( fontLabels );
		widget->graph( 0 )->setPen( plotPen );
		widget->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom ); //allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
	};

	inline void setupSecondGraph( QString ylabel1, QString ylabel2 )
	{
		widget->addGraph( widget->xAxis, widget->yAxis2 );
		widget->yAxis2->setVisible( true );
		widget->yAxis2->setLabel( ylabel2 );
		widget->yAxis2->setTickLabelFont( fontTicks );
		widget->yAxis2->setLabelFont( fontLabels );
		widget->graph( 1 )->setPen( plotPen2 );
		widget->legend->setVisible( true );
		widget->axisRect()->insetLayout()->setInsetAlignment( 0, Qt::AlignBottom | Qt::AlignRight );
		widget->graph( 0 )->setName( ylabel1 );
		widget->graph( 1 )->setName( ylabel2 );
		//widget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
		//widget->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
	}

	inline void setupMultipleGraph( std::vector<std::string> ylabels )
	{
		for ( int i = 0; i < ylabels.size(); i++ )
		{
			if ( i )
				widget->addGraph();

			if ( i < plotPens.size() )
				widget->graph( i )->setPen( plotPens[i] );
			else
				widget->graph( i )->setPen( QPen( Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );

			widget->graph( i )->setName( QString::fromStdString( ylabels[i] ) );

			//widget->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
		}
		widget->axisRect()->insetLayout()->setInsetAlignment( 0, Qt::AlignBottom | Qt::AlignRight );
		widget->legend->setVisible( true );
	}

	inline void setAxisNames( std::string xlabel, std::string ylabel ) override
	{
		widget->xAxis->setLabel( QString::fromStdString( xlabel ) );
		widget->yAxis->setLabel( QString::fromStdString( ylabel ) );
	}

	inline void setAxisNames( std::string xlabel, std::string ylabel1, std::string ylabel2 ) override
	{
		widget->xAxis->setLabel( QString::fromStdString( xlabel ) );
		widget->yAxis->setLabel( QString::fromStdString( ylabel1 ) );
		widget->yAxis2->setLabel( QString::fromStdString( ylabel2 ) );
		if ( widget->graphCount() < 2 )
			setupSecondGraph( QString::fromStdString( ylabel1 ), QString::fromStdString( ylabel2 ) );
	}

	inline void setAxisNames( std::string xlabel, std::string ylabel, std::vector<std::string> ylabels ) override
	{
		widget->xAxis->setLabel( QString::fromStdString( xlabel ) );
		widget->yAxis->setLabel( QString::fromStdString( ylabel ) );
		if ( widget->graphCount() < ylabels.size() )
			setupMultipleGraph( ylabels );
	}

	inline void plot( const std::vector<double> &y ) override
	{
		std::vector<double> iotam( y.size() );
		std::iota( iotam.begin(), iotam.end(), 0 );
		widget->graph( 0 )->setData( QVector<double>::fromStdVector( iotam ), QVector<double>::fromStdVector( y ) );
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot( const std::vector<double> &x, const std::vector<double> &y ) override
	{
		widget->graph( 0 )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( y ) );
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot( const std::vector<double> &x, const std::vector<double> &y1, const std::vector<double> &y2 ) override
	{
		widget->graph( 0 )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( y1 ) );
		widget->graph( 1 )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( y2 ) );
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot( const std::vector<double> &x, const std::vector<std::vector<double>> &ys ) override
	{
		for ( int i = 0; i < ys.size(); i++ )
		{
			widget->graph( i )->setData( QVector<double>::fromStdVector( x ), QVector<double>::fromStdVector( ys[i] ) );
		}
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot( const double x, const double y ) override
	{
		widget->graph( 0 )->addData( x, y );
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot( const double x, const double y1, const double y2 ) override
	{
		widget->graph( 0 )->addData( x, y1 );
		widget->graph( 1 )->addData( x, y2 );
		widget->rescaleAxes();
		widget->replot();
	}

	inline void clear( bool second ) override
	{
		widget->graph( 0 )->data()->clear();
		if ( second )
			widget->graph( 1 )->data()->clear();
	}

	inline void save( std::string path ) override
	{
		widget->savePng( QString::fromStdString( path ), 0, 0, 3, -1 );
	}
};
