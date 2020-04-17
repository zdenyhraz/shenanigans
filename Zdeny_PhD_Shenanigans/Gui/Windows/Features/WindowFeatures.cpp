#include "stdafx.h"
#include "WindowFeatures.h"

WindowFeatures::WindowFeatures( QWidget *parent, Globals *globals ) : QMainWindow( parent ), globals( globals )
{
	ui.setupUi( this );
}

