#include "stdafx.h"
#include "WindowDiffrot.h"
#include "Astrophysics/diffrot.h"


WindowDiffrot::WindowDiffrot( QWidget *parent, Globals *globals ) : QMainWindow( parent ), globals( globals )
{
	ui.setupUi( this );
	diffrotResults = new DiffrotResults;
	connect( ui.pushButton, SIGNAL( clicked() ), this, SLOT( calculateDiffrot() ) );
	connect( ui.pushButton_2, SIGNAL( clicked() ), this, SLOT( showResults() ) );
	connect( ui.pushButton_3, SIGNAL( clicked() ), this, SLOT( showIPC() ) );
	connect( ui.pushButton_5, SIGNAL( clicked() ), this, SLOT( checkDiskShifts() ) );
}

void WindowDiffrot::calculateDiffrot()
{
	LOG_INFO( "Calculating diffrot profile..." );
	FitsTime fitsTime( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );

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
	drset.visual = ui.checkBox_3->isChecked();
	drset.savepath = ui.lineEdit_9->text().toStdString();

	*diffrotResults = calculateDiffrotProfile( *globals->IPCset, fitsTime, drset );
	LOG_SUCC( "Differential rotation profile calculated." );
}

void WindowDiffrot::showResults()
{
	if ( diffrotResults->calculated )
		diffrotResults->ShowResults( ui.lineEdit_20->text().toDouble(), ui.lineEdit_16->text().toDouble(), ui.lineEdit_18->text().toDouble(), ui.lineEdit_19->text().toDouble() );
	else
		LOG_DEBUG( "Diffrot results not yet calculated!" );
}

void WindowDiffrot::showIPC()
{
	LOG_INFO( "Showimg diffrot profile IPC landscape..." );
	FitsParams params1, params2;
	IPCsettings set = *globals->IPCset;
	set.broadcast = true;
	//set.save = true;
	FitsTime fitsTime( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );
	LOG_INFO( "Loading file '" + fitsTime.path() + "'..." );
	Mat pic1 = roicrop( loadfits( fitsTime.path(), params1 ), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows() );
	fitsTime.advanceTime( ui.lineEdit_5->text().toDouble()*ui.lineEdit_8->text().toDouble() );
	LOG_INFO( "Loading file '" + fitsTime.path() + "'..." );
	Mat pic2 = roicrop( loadfits( fitsTime.path(), params1 ), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows() );

	auto shifts = phasecorrel( pic1, pic2, set );
}

void WindowDiffrot::checkDiskShifts()
{
	LOG_INFO( "Checking diffrot disk shifts..." );
	FitsTime fitsTime( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );
	FitsParams params1, params2;

	// NSWE shifts
	IPCsettings set = *globals->IPCset;
	set.broadcast = true;

	// fulldisk shift
	IPCsettings setG = *globals->IPCset;
	setG.broadcast = true;
	setG.setSize( 4096, 4096 );

	LOG_INFO( "Loading file '" + fitsTime.path() + "'..." );
	Mat pic1 = loadfits( fitsTime.path(), params1 );

	fitsTime.advanceTime( ui.lineEdit_5->text().toDouble()*ui.lineEdit_8->text().toDouble() );

	LOG_INFO( "Loading file '" + fitsTime.path() + "'..." );
	Mat pic2 = loadfits( fitsTime.path(), params1 );

	int edgeN = 0.025 * 4096;
	int edgeS = 0.974 * 4096;
	int edgeW = 0.027 * 4096;
	int edgeE = 0.975 * 4096;
	int center = 0.5 * 4096;

	std::vector<Mat> pics( 4 );
	pics[0] = roicrop( pic1, center, edgeN, set.getcols(), set.getrows() );
	pics[3] = roicrop( pic1, center, edgeS, set.getcols(), set.getrows() );
	pics[2] = roicrop( pic1, edgeW, center, set.getcols(), set.getrows() );
	pics[1] = roicrop( pic1, edgeE, center, set.getcols(), set.getrows() );

	showimg( pics, "pics" );

	//auto shiftsG = phasecorrel( pic1, pic2, setG );
	auto shiftsN = phasecorrel( roicrop( pic1, center, edgeN, set.getcols(), set.getrows() ), roicrop( pic2, center, edgeN, set.getcols(), set.getrows() ), set );
	auto shiftsS = phasecorrel( roicrop( pic1, center, edgeS, set.getcols(), set.getrows() ), roicrop( pic2, center, edgeS, set.getcols(), set.getrows() ), set );
	auto shiftsW = phasecorrel( roicrop( pic1, edgeW, center, set.getcols(), set.getrows() ), roicrop( pic2, edgeW, center, set.getcols(), set.getrows() ), set );
	auto shiftsE = phasecorrel( roicrop( pic1, edgeE, center, set.getcols(), set.getrows() ), roicrop( pic2, edgeE, center, set.getcols(), set.getrows() ), set );

	//LOG_INFO( "Diffrot shifts G = [{},{}]", shiftsG.x, shiftsG.y );
	LOG_INFO( "Diffrot shifts N = {}", shiftsN.y );
	LOG_INFO( "Diffrot shifts S = {}", shiftsS.y );
	LOG_INFO( "Diffrot shifts W = {}", shiftsW.x );
	LOG_INFO( "Diffrot shifts E = {}", shiftsE.x );


}




