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






	LOG_INFO( "Finished matching features" );
}