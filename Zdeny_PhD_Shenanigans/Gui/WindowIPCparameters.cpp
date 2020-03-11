#include "stdafx.h"
#include "WindowIPCparameters.h"

WindowIPCparameters::WindowIPCparameters( QWidget *parent, Globals *globals ) : QMainWindow( parent ), globals( globals )
{
	ui.setupUi( this );
	this->refreshIPCparameters( true );
	connect( ui.pushButton, SIGNAL( clicked() ), this, SLOT( refreshIPCparametersAndExit() ) );
	connect( ui.pushButton_2, SIGNAL( clicked() ), this, SLOT( refreshIPCparameters() ) );
}

void WindowIPCparameters::refreshIPCparameters( bool init )
{
	globals->IPCset->setSize( ui.lineEdit->text().toInt(), ui.lineEdit_9->text().toInt() );
	globals->IPCset->L2size = ui.lineEdit_2->text().toInt();
	globals->IPCset->L1ratio = ui.lineEdit_3->text().toDouble();
	globals->IPCset->UC = ui.lineEdit_4->text().toInt();
	globals->IPCset->setBandpassParameters( ui.lineEdit_5->text().toDouble(), ui.lineEdit_6->text().toDouble() );
	globals->IPCset->epsilon = ui.lineEdit_7->text().toDouble();
	globals->IPCset->minimalShift = ui.lineEdit_8->text().toDouble();

	globals->IPCset->subpixel = ui.checkBox->isChecked();
	globals->IPCset->applyBandpass = ui.checkBox_3->isChecked();
	globals->IPCset->applyWindow = ui.checkBox_4->isChecked();
	globals->IPCset->interpolate = ui.checkBox_5->isChecked();
	globals->IPCset->crossCorrel = ui.checkBox_7->isChecked();

	if ( !init )
		LOG_DEBUG( "IPC parameter values updated" );
}

void WindowIPCparameters::refreshIPCparametersAndExit()
{
	this->refreshIPCparameters();
	this->hide();
}
