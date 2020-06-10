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

	int edgeN = 0.025 * 4096;
	int edgeS = 0.974 * 4096;
	int edgeW = 0.027 * 4096;
	int edgeE = 0.975 * 4096;
	int center = 0.5 * 4096;

	int pics = ui.lineEdit_7->text().toDouble();
	std::vector<double> shiftsN;
	std::vector<double> shiftsS;
	std::vector<double> shiftsW;
	std::vector<double> shiftsE;
	std::vector<double> shiftsFX;
	std::vector<double> shiftsFY;
	FitsImage pic1, pic2;
	int lag1, lag2;
	Mat picshow;

	for ( int pic = 0; pic < pics; pic++ )
	{
		LOG_DEBUG( "{} / {} ...", pic + 1, pics );
		time.advanceTime( ( bool )pic * ( ui.lineEdit_6->text().toDouble() - ui.lineEdit_5->text().toDouble() ) * ui.lineEdit_8->text().toDouble() );
		loadFitsFuzzy( pic1, time, lag1 );
		time.advanceTime( ui.lineEdit_5->text().toDouble() * ui.lineEdit_8->text().toDouble() );
		loadFitsFuzzy( pic2, time, lag2 );

		if ( !pic ) picshow = pic1.image().clone();

		if ( pic1.params().succload && pic2.params().succload )
		{
			shiftsN.push_back( phasecorrel( roicrop( pic1.image(), center, edgeN, set.getcols(), set.getrows() ), roicrop( pic2.image(), center, edgeN, set.getcols(), set.getrows() ), set ).y );
			shiftsS.push_back( phasecorrel( roicrop( pic1.image(), center, edgeS, set.getcols(), set.getrows() ), roicrop( pic2.image(), center, edgeS, set.getcols(), set.getrows() ), set ).y );
			shiftsW.push_back( phasecorrel( roicrop( pic1.image(), edgeW, center, set.getcols(), set.getrows() ), roicrop( pic2.image(), edgeW, center, set.getcols(), set.getrows() ), set ).x );
			shiftsE.push_back( phasecorrel( roicrop( pic1.image(), edgeE, center, set.getcols(), set.getrows() ), roicrop( pic2.image(), edgeE, center, set.getcols(), set.getrows() ), set ).x );
			shiftsFX.push_back( pic2.params().fitsMidX - pic1.params().fitsMidX );
			shiftsFY.push_back( pic2.params().fitsMidY - pic1.params().fitsMidY );
		}
	}

	LOG_INFO( "<<<<<<<<<<<<<<<<<<   IPC median   /   FITS median   /   ABSDIFF median   >>>>>>>>>>>>>>>>>>>>>" );
	LOG_INFO( "Diffrot shifts N = {} / {} / {}", median( shiftsN ), median( shiftsFY ), median( abs( shiftsN - shiftsFY ) ) );
	LOG_INFO( "Diffrot shifts S = {} / {} / {}", median( shiftsS ), median( shiftsFY ), median( abs( shiftsS - shiftsFY ) ) );
	LOG_INFO( "Diffrot shifts W = {} / {} / {}", median( shiftsW ), median( shiftsFX ), median( abs( shiftsW - shiftsFX ) ) );
	LOG_INFO( "Diffrot shifts E = {} / {} / {}", median( shiftsE ), median( shiftsFX ), median( abs( shiftsE - shiftsFX ) ) );

	LOG_INFO( "<<<<<<<<<<<<<<<<<<   IPC mean   /   FITS mean   /   ABSDIFF mean   >>>>>>>>>>>>>>>>>>>>>" );
	LOG_INFO( "Diffrot shifts N = {} / {} / {}", mean( shiftsN ), mean( shiftsFY ), mean( abs( shiftsN - shiftsFY ) ) );
	LOG_INFO( "Diffrot shifts S = {} / {} / {}", mean( shiftsS ), mean( shiftsFY ), mean( abs( shiftsS - shiftsFY ) ) );
	LOG_INFO( "Diffrot shifts W = {} / {} / {}", mean( shiftsW ), mean( shiftsFX ), mean( abs( shiftsW - shiftsFX ) ) );
	LOG_INFO( "Diffrot shifts E = {} / {} / {}", mean( shiftsE ), mean( shiftsFX ), mean( abs( shiftsE - shiftsFX ) ) );

	std::vector<Mat> picsshow( 4 );
	picsshow[0] = roicrop( picshow, center, edgeN, set.getcols(), set.getrows() );
	picsshow[3] = roicrop( picshow, center, edgeS, set.getcols(), set.getrows() );
	picsshow[2] = roicrop( picshow, edgeW, center, set.getcols(), set.getrows() );
	picsshow[1] = roicrop( picshow, edgeE, center, set.getcols(), set.getrows() );
	showimg( picsshow, "pics" );

	std::vector<double> iotam( shiftsFX.size() );
	std::iota( iotam.begin(), iotam.end(), 0 );
	iotam = ( double )( ui.lineEdit_7->text().toDouble() - 1 ) * ui.lineEdit_6->text().toDouble() * 45 / 60 / 60 / 24 / ( iotam.size() - 1 ) * iotam;
	Plot1D::plot( iotam, std::vector<std::vector<double>> {shiftsFX, shiftsW, shiftsE}, "shiftsX", "time [days]", "45sec shiftX [px]", std::vector<std::string> {"shifts fits header X", "shifts IPC west edge", "shifts IPC east edge"} );
	Plot1D::plot( iotam, std::vector<std::vector<double>> {shiftsFY, shiftsN, shiftsS}, "shiftsY", "time [days]", "45sec shiftY [px]", std::vector<std::string> {"shifts fits header Y", "shifts IPC north edge", "shifts IPC south edge"} );

}




