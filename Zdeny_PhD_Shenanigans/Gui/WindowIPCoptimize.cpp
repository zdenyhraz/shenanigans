#include "stdafx.h"
#include "WindowIPCoptimize.h"
#include "WindowIPCparameters.h"
#include "Astrophysics/functionsAstro.h"
#include "Plot/plotterqt1D.h"

WindowIPCoptimize::WindowIPCoptimize( QWidget *parent, Globals *globals ) : QMainWindow( parent ), globals( globals )
{
	ui.setupUi( this );
	connect( ui.pushButton, SIGNAL( clicked() ), this, SLOT( optimize() ) );
	connect( ui.pushButton_2, SIGNAL( clicked() ), this, SLOT( optimizeAll() ) );
}

void WindowIPCoptimize::optimize()
{
	if ( 0 ) //dbg
	{
		Evolution Evo( 2 );
		Evo.NP = 32;
		Evo.lowerBounds = zerovect( 2, -100 );
		Evo.upperBounds = zerovect( 2, +100 );
		Evo.optimize( [&]( std::vector<double> arg ) {return sin( sqr( arg[0] ) - sqr( arg[1] - 3 ) + 6 ); } );
	}
	Plot1D plt( globals->widget1 );
	optimizeIPCParameters( *globals->IPCset, ui.lineEdit->text().toStdString(), ui.lineEdit_2->text().toStdString(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_4->text().toDouble(),
	                       ui.lineEdit_5->text().toInt(),  &plt );
	LOG_DEBUG( "IPC parameter optimization completed, see the results at\n" + ui.lineEdit_2->text().toStdString() );
}

void WindowIPCoptimize::optimizeAll()
{
	optimizeIPCParametersForAllWavelengths( *globals->IPCset, ui.lineEdit_3->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_5->text().toInt() );
}