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

	std::string path1 = ui.lineEdit->text().toStdString();
	std::string path2 = ui.lineEdit_2->text().toStdString();

	FeatureMatchData data;
	data.ftype = ( FeatureType )ui.comboBox->currentIndex();
	data.thresh = ui.lineEdit_4->text().toDouble();
	data.matchcnt = ui.lineEdit_3->text().toInt();
	data.magnitudeweight = ui.lineEdit_5->text().toDouble();
	data.quanB = ui.lineEdit_6->text().toDouble();
	data.quanT = ui.lineEdit_7->text().toDouble();

	featureMatch( path1, path2, data );

	LOG_INFO( "Finished matching features" );
}