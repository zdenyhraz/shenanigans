#include "stdafx.h"
#include "WindowFeatures.h"

WindowFeatures::WindowFeatures( QWidget *parent, Globals *globals ) : QMainWindow( parent ), globals( globals )
{
	ui.setupUi( this );
	connect( ui.pushButton, SIGNAL( clicked() ), this, SLOT( FeatureMatch() ) );
}

void WindowFeatures::FeatureMatch()
{
	LOG_INFO( "Matching features..." );

	FeatureMatchData data;
	data.path1 = ui.lineEdit->text().toStdString();
	data.path2 = ui.lineEdit_2->text().toStdString();
	data.path = ui.lineEdit_8->text().toStdString();
	data.ftype = ( FeatureType )ui.comboBox->currentIndex();
	data.thresh = ui.lineEdit_4->text().toDouble();
	data.matchcnt = ui.lineEdit_3->text().toInt();
	data.magnitudeweight = ui.lineEdit_5->text().toDouble();
	data.quanB = ui.lineEdit_6->text().toDouble();
	data.quanT = ui.lineEdit_7->text().toDouble();
	data.degree = ui.lineEdit_9->text().toInt();
	data.proxpts = ui.lineEdit_10->text().toInt();
	data.proxcoeff = ui.lineEdit_11->text().toDouble();
	data.pathout = ui.lineEdit_12->text().toStdString();

	featureMatch( data );

	LOG_INFO( "Finished matching features" );
}