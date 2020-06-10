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
	FitsTime time( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );

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

	*diffrotResults = calculateDiffrotProfile( *globals->IPCset, time, drset );
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
	FitsTime time( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );
	LOG_INFO( "Loading file '" + time.path() + "'..." );
	Mat pic1 = roicrop( loadfits( time.path(), params1 ), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows() );
	time.advanceTime( ui.lineEdit_5->text().toDouble()*ui.lineEdit_8->text().toDouble() );
	LOG_INFO( "Loading file '" + time.path() + "'..." );
	Mat pic2 = roicrop( loadfits( time.path(), params1 ), params1.fitsMidX, params1.fitsMidY, set.getcols(), set.getrows() );

	auto shifts = phasecorrel( pic1, pic2, set );
}

void WindowDiffrot::checkDiskShifts()
{
	LOG_INFO( "Checking diffrot disk shifts..." );
	FitsTime time( ui.lineEdit_17->text().toStdString(), ui.lineEdit_10->text().toInt(), ui.lineEdit_11->text().toInt(), ui.lineEdit_12->text().toInt(), ui.lineEdit_13->text().toInt(), ui.lineEdit_14->text().toInt(), ui.lineEdit_15->text().toInt() );
	IPCsettings set = *globals->IPCset;
	//set.broadcast = true;

	int edgeN = 0.025 * 4096;
	int edgeS = 0.974 * 4096;
	int edgeW = 0.027 * 4096;
	int edgeE = 0.975 * 4096;
	int center = 0.5 * 4096;

	int pics = 11;
	std::vector<double> shiftsN( pics );
	std::vector<double> shiftsS( pics );
	std::vector<double> shiftsW( pics );
	std::vector<double> shiftsE( pics );
	std::vector<double> shiftsFX( pics );
	std::vector<double> shiftsFY( pics );
	std::vector<double> iotam( pics );
	FitsImage pic1, pic2;
	int lag1, lag2;
	Mat picshow;

	for ( int pic = 0; pic < pics; pic++ )
	{
		time.advanceTime( ( bool )pic * ( ui.lineEdit_6->text().toDouble() - ui.lineEdit_5->text().toDouble() ) * ui.lineEdit_8->text().toDouble() );
		loadFitsFuzzy( pic1, time, lag1 );
		time.advanceTime( ui.lineEdit_5->text().toDouble() * ui.lineEdit_8->text().toDouble() );
		loadFitsFuzzy( pic2, time, lag2 );

		if ( !pic ) picshow = pic1.image().clone();

		iotam[pic] = pic + 1;
		shiftsN[pic] = phasecorrel( roicrop( pic1.image(), center, edgeN, set.getcols(), set.getrows() ), roicrop( pic2.image(), center, edgeN, set.getcols(), set.getrows() ), set ).y;
		shiftsS[pic] = phasecorrel( roicrop( pic1.image(), center, edgeS, set.getcols(), set.getrows() ), roicrop( pic2.image(), center, edgeS, set.getcols(), set.getrows() ), set ).y;
		shiftsW[pic] = phasecorrel( roicrop( pic1.image(), edgeW, center, set.getcols(), set.getrows() ), roicrop( pic2.image(), edgeW, center, set.getcols(), set.getrows() ), set ).x;
		shiftsE[pic] = phasecorrel( roicrop( pic1.image(), edgeE, center, set.getcols(), set.getrows() ), roicrop( pic2.image(), edgeE, center, set.getcols(), set.getrows() ), set ).x;
		shiftsFX[pic] = pic2.params().fitsMidX - pic1.params().fitsMidX;
		shiftsFY[pic] = pic2.params().fitsMidY - pic1.params().fitsMidY;
	}

	std::vector<Mat> picsshow( 4 );
	picsshow[0] = roicrop( picshow, center, edgeN, set.getcols(), set.getrows() );
	picsshow[3] = roicrop( picshow, center, edgeS, set.getcols(), set.getrows() );
	picsshow[2] = roicrop( picshow, edgeW, center, set.getcols(), set.getrows() );
	picsshow[1] = roicrop( picshow, edgeE, center, set.getcols(), set.getrows() );
	showimg( picsshow, "pics" );

	LOG_INFO( "<<<<<<<<<<<<<<<<<<   IPC   /   FITS   /   DIFF   >>>>>>>>>>>>>>>>>>>>>" );
	LOG_INFO( "Diffrot shifts N = {} / {} / {}", median( shiftsN ), median( shiftsFY ), median( shiftsN - shiftsFY ) );
	LOG_INFO( "Diffrot shifts S = {} / {} / {}", median( shiftsS ), median( shiftsFY ), median( shiftsS - shiftsFY ) );
	LOG_INFO( "Diffrot shifts W = {} / {} / {}", median( shiftsW ), median( shiftsFX ), median( shiftsW - shiftsFX ) );
	LOG_INFO( "Diffrot shifts E = {} / {} / {}", median( shiftsE ), median( shiftsFX ), median( shiftsE - shiftsFX ) );

	Plot1D::plot( iotam, std::vector<std::vector<double>> {shiftsFX, shiftsW, shiftsE}, "shiftsX", "pic", "shiftX", std::vector<std::string> {"shiftsFX", "shiftsW", "shiftsE"} );
	Plot1D::plot( iotam, std::vector<std::vector<double>> {shiftsFY, shiftsN, shiftsS}, "shiftsY", "pic", "shiftY", std::vector<std::string> {"shiftsFY", "shiftsN", "shiftsS"} );

}




