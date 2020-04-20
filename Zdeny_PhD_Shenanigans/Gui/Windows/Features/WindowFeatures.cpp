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
	int featurecount = ui.lineEdit_3->text().toInt();
	FeatureType ftype = ( FeatureType )ui.comboBox->currentIndex();

	featureMatch( path1, path2, featurecount, ftype );


	LOG_INFO( "Finished matching features" );
}