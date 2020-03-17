#include "stdafx.h"
#include "WindowDiffrot.h"
#include "Astrophysics/diffrot.h"
#include "Plot/Plot1DImpl.h"

WindowDiffrot::WindowDiffrot( QWidget *parent, Globals *globals ) : QMainWindow( parent ), globals( globals )
{
	ui.setupUi( this );
	diffrotResults = new DiffrotResults;
	connect( ui.pushButton, SIGNAL( clicked() ), this, SLOT( calculateDiffrot() ) );
	connect( ui.pushButton_2, SIGNAL( clicked() ), this, SLOT( showResults() ) );
	connect( ui.pushButton_3, SIGNAL( clicked() ), this, SLOT( showIPC() ) );
	connect( ui.pushButton_4, SIGNAL( clicked() ), this, SLOT( optimizeDiffrot() ) );
	connect( ui.pushButton_5, SIGNAL( clicked() ), this, SLOT( superOptimizeDiffrot() ) );
}

void WindowDiffrot::calculateDiffrot()
{
	LOG_INFO( "Calculating diffrot profile..." );
	FitsTime fitsTime( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );
	Plot1Di pltX( globals->widget1 );
	Plot1Di pltY( globals->widget2 );
	//*diffrotResults = calculateDiffrotProfile(*globals->IPCset, fitsTime, ui.lineEdit_7->text().toDouble(), ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(),  &pltX, &pltY);

	DiffrotSettings drset;
	drset.pics = ui.lineEdit_7->text().toDouble();
	drset.ys = ui.lineEdit_2->text().toDouble();
	drset.sPic = ui.lineEdit_6->text().toDouble();
	drset.dPic = ui.lineEdit_5->text().toDouble();
	drset.vFov = ui.lineEdit_4->text().toDouble();
	drset.dSec = ui.lineEdit_8->text().toDouble();
	drset.medianFilter = ui.checkBox->isChecked();
	drset.movavgFilter = ui.checkBox_2->isChecked();
	drset.medianFilterSize = ui.lineEdit_21->text().toDouble();
	drset.movavgFilterSize = ui.lineEdit_22->text().toDouble();

	*diffrotResults = calculateDiffrotProfile( *globals->IPCset, fitsTime, drset,  &pltX, &pltY );
	LOG_SUCC( "Differential rotation profile calculated." );
}

void WindowDiffrot::showResults()
{
	diffrotResults->ShowResults( ui.lineEdit_20->text().toDouble(), ui.lineEdit_16->text().toDouble(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble() );
	LOG_INFO( "Differential rotation results shown" );
}

void WindowDiffrot::showIPC()
{
	LOG_INFO( "Showimg diffrot profile IPC landscape..." );
	FitsParams params1, params2;
	IPCsettings set = *globals->IPCset;
	set.broadcast = true;
	FitsTime fitsTime( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(),
	                   ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );
	LOG_INFO( "Loading file '" + fitsTime.path() + "'..." );
	Mat pic1 = roicrop( loadfits( fitsTime.path(), params1 ), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows() );
	fitsTime.advanceTime( ui.lineEdit_5->text().toDouble()*ui.lineEdit_8->text().toDouble() );
	LOG_INFO( "Loading file '" + fitsTime.path() + "'..." );
	Mat pic2 = roicrop( loadfits( fitsTime.path(), params1 ), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows() );

	auto shifts = phasecorrel( pic1, pic2, set );
}

void WindowDiffrot::optimizeDiffrot()
{
	FitsParams params1, params2;
	FitsTime fitsTime( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );
	std::vector<int> sizes{ 16, 32, 64, 128 };
	std::string path = "D:\\MainOutput\\diffrot\\diffrotIPCopt.csv";
	std::ofstream listing( path, std::ios::out | std::ios::trunc ); //just delete
	for ( auto &size : sizes )
	{
		LOG_INFO( "Optimizing IPC parameters for diffrot profile measurement " + to_string( size ) + "x" + to_string( size ) + "..." );
		IPCsettings set = *globals->IPCset;
		set.setSize( size, size );
		Plot1Di *plt = new Plot1Di( globals->widget1 );
		optimizeIPCParameters( set, fitsTime.path(), path, 5, 0.01, 3,  plt );
		delete plt;
	}
	LOG_INFO( "IPC parameters optimization for diffrot profile measurement finished" );
}

void WindowDiffrot::superOptimizeDiffrot()
{
	/*
	// size, Lm, Hm, L2, W
	Evolution Evo( 5 );
	Evo.lowerBounds = std::vector<double> { 10, 0, 0, 5, -1 };
	Evo.upperBounds = std::vector<double> { 256, 10, 500, 15, 1 };
	*/

	// Lm, Hm
	Evolution Evo( 2 );
	Evo.lowerBounds = std::vector<double> {  0, 0 };
	Evo.upperBounds = std::vector<double> { 10, 500 };

	Evo.NP = 50;
	Evo.mutStrat = Evolution::MutationStrategy::BEST1;

	LOG_INFO( "Super optimizing diffrot profile..." );
	Plot1Di *plt1 = new Plot1Di( globals->widget1 );
	Plot1Di *plt2 = new Plot1Di( globals->widget2 );

	FitsTime fitsTimeMaster( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(),
	                         ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );

	std::vector<std::pair<FitsImage, FitsImage>> pics;
	double dt = 45;
	for ( int i = 0; i < 3; i++ )
	{
		FitsImage a( fitsTimeMaster.path() );
		fitsTimeMaster.advanceTime( dt );

		FitsImage b( fitsTimeMaster.path() );

		if ( a.params().succload && b.params().succload )
			pics.emplace_back( std::make_pair( a, b ) );
	}

	//auto f = [&]( std::vector<double> arg ) { return DiffrotMerritFunctionWrapper( arg, pics, ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(), plt2 ); };
	auto f2 = [&]( std::vector<double> arg ) { return DiffrotMerritFunctionWrapper2( arg, pics, ui.lineEdit->text().toDouble(), ui.lineEdit_2->text().toDouble(), ui.lineEdit_3->text().toDouble(), ui.lineEdit_6->text().toDouble(), ui.lineEdit_5->text().toDouble(), ui.lineEdit_4->text().toDouble(), ui.lineEdit_8->text().toDouble(), plt2 ); };

	auto result = Evo.optimize( f2 );
	LOG_SUCC( "Optimal Lmult: " + to_string( result[0] ) );
	LOG_SUCC( "Optimal Hmult: " + to_string( result[1] ) );
	LOG_INFO( "Super optimizing finished for dt={}`", dt );
}

