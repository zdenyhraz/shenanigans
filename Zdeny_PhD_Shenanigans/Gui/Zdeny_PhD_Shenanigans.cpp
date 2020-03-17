#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"

Zdeny_PhD_Shenanigans::Zdeny_PhD_Shenanigans( QWidget *parent ) : QMainWindow( parent )
{
	ui.setupUi( this );

	//show the logo
	QPixmap pm( "Resources/logo.png" ); // <- path to image file
	ui.label_2->setPixmap( pm );
	ui.label_2->setScaledContents( true );

	//allocate all globals - main window is loaded once only
	globals = new Globals();
	globals->widget1 = ui.widget;
	globals->widget2 = ui.widget_2;

	//create all windows
	windowIPCparameters = new WindowIPCparameters( this, globals );
	windowIPCoptimize = new WindowIPCoptimize( this, globals );
	windowIPC2PicAlign = new WindowIPC2PicAlign( this, globals );
	windowDiffrot = new WindowDiffrot( this, globals );

	//make signal to slot connections
	connect( ui.actionExit, SIGNAL( triggered() ), this, SLOT( exit() ) );
	connect( ui.actionAbout_Zdeny_s_PhD_Shenanigans, SIGNAL( triggered() ), this, SLOT( about() ) );
	connect( ui.pushButtonClose, SIGNAL( clicked() ), this, SLOT( closeCV() ) );
	connect( ui.actionIPC_parameters, SIGNAL( triggered() ), this, SLOT( showWindowIPCparameters() ) );
	connect( ui.actionIPC_optimize, SIGNAL( triggered() ), this, SLOT( showWindowIPCoptimize() ) );
	connect( ui.actionIPC_2pic_align, SIGNAL( triggered() ), this, SLOT( showWindowIPC2PicAlign() ) );
	connect( ui.actionDebug, SIGNAL( triggered() ), this, SLOT( debug() ) );
	connect( ui.pushButtonDebug, SIGNAL( clicked() ), this, SLOT( debug() ) );
	connect( ui.actiondiffrot, SIGNAL( triggered() ), this, SLOT( showWindowDiffrot() ) );
	connect( ui.actionPlay, SIGNAL( triggered() ), this, SLOT( playSnake() ) );
	connect( ui.actionGenerate_land, SIGNAL( triggered() ), this, SLOT( generateLand() ) );
	connect( ui.actionFits_downloader, SIGNAL( triggered() ), this, SLOT( fitsDownloader() ) );

	LOG_SUCC( "Welcome back, my friend." ); //log a welcome message
}

void Zdeny_PhD_Shenanigans::exit()
{
	QApplication::exit();
}

void Zdeny_PhD_Shenanigans::debug()
{
	if ( 0 ) //1D plot
	{
		int n = 10001;
		auto x = zerovect( n );
		auto y = zerovect( n );
		for ( int i = 0; i < n; i++ )
		{
			x[i] = ( double )i / ( n - 1 );
			y[i] = sin( 2 * Constants::Pi * x[i] );
		}

		Plot1Di plt( ui.widget );
		plt.plot( x, y );
	}
	if ( 0 ) //plot in optimization
	{
		Evolution Evo( 2 );
		Evo.NP = 10;
		Plot1Di plt( ui.widget );
		auto f = [&]( std::vector<double> args )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
			return abs( args[0] - args[1] );
		};
		auto result = Evo.optimize( f, &plt );
		plt.save( "D:\\MainOutput\\Debug\\plot1D.png" );
	}
	if ( 0 ) //ipc bandpass & window
	{
		IPCsettings set = *globals->IPCset;
		set.setSize( 1000, 1000 );
		set.setBandpassParameters( 5, 1 );
		Plot2D::plot( set.bandpass, "x", "y", "z", 0, 1, 0, 1 );
	}
	if ( 0 ) //2pic IPC
	{
		std::string path1 = "C:\\Users\\Zdeny\\Documents\\wp\\cat_gray_glance_154511_3840x2160.jpg";
		std::string path2 = "C:\\Users\\Zdeny\\Documents\\wp\\cat_gray_glance_154511_3840x2160.jpg";
		Mat img1 = loadImage( path1 );
		Mat img2 = loadImage( path2 );

		double shiftX = 10.3;
		double shiftY = -7.2;
		Mat T = ( Mat_<float>( 2, 3 ) << 1., 0., shiftX, 0., 1., shiftY );
		warpAffine( img2, img2, T, cv::Size( img2.cols, img2.rows ) );

		IPCsettings set = *globals->IPCset;
		set.broadcast = true;
		set.setSize( img1.rows, img1.cols );

		auto shifts = phasecorrel( img1, img2, set );
	}
	if ( 0 ) //Plot2D test
	{
		int Ny = 1000;
		int Nx = 1000;
		auto Z = zerovect2( Ny, Nx );

		for ( int y = 0; y < Ny; y++ )
		{
			for ( int x = 0; x < Nx; x++ )
			{
				//Z[y][x] = ( x - y ) * ( x - y );
				Z[y][x] = rand();
			}
		}

		Plot2D::plot( Z, "niceplot", "x", "y", "z" );
	}
	if ( 1 ) //Plot1D test
	{
		int N = 1000;
		auto Y = zerovect( N );

		for ( int x = 0; x < N; x++ )
		{
			Y[x] = rand();
		}

		Plot1D::plot( Y, "niceplot", "x", "y" );
	}

	LOG_INFO( "Debug finished." );
}

void Zdeny_PhD_Shenanigans::about()
{
	QMessageBox msgBox;
	msgBox.setText( "All these shenanigans were created during my PhD studies.\n\nHave fun,\nZdenek Hrazdira\n2018-2020" );
	msgBox.exec();
}

void Zdeny_PhD_Shenanigans::closeCV()
{
	destroyAllWindows();
	LOG_INFO( "All image windows closed" );
}

void Zdeny_PhD_Shenanigans::showWindowIPCparameters()
{
	windowIPCparameters->show();
}

void Zdeny_PhD_Shenanigans::showWindowIPCoptimize()
{
	windowIPCoptimize->show();
}

void Zdeny_PhD_Shenanigans::showWindowIPC2PicAlign()
{
	windowIPCparameters->show();
	windowIPC2PicAlign->show();
}

void Zdeny_PhD_Shenanigans::showWindowDiffrot()
{
	windowDiffrot->show();
}

void Zdeny_PhD_Shenanigans::generateLand()
{
	LOG_INFO( "Generating some land..." );
	Mat mat = procedural( 1000, 1000 );
	showimg( colorlandscape( mat ), "procedural nature" );
	//showimg(mat, "procedural mature", true);
	LOG_INFO( "Finished generating some land. Do you like it?" );
}

void Zdeny_PhD_Shenanigans::playSnake()
{
	LOG_INFO( "Started playing snake. (It is ok, everyone needs some rest.. :))" );
	SnakeGame();
	LOG_INFO( "Finished playing snake. Did you enjoy it? *wink*" );
}

void Zdeny_PhD_Shenanigans::fitsDownloader()
{
	//http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122 - 2020 1.1. 00:00
	generateFitsDownloadUrlSingles( 1, 2000, "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122" );
	LOG_INFO( "Fits download urls created" );
}

void Zdeny_PhD_Shenanigans::closeEvent( QCloseEvent *event )
{
	QMessageBox::StandardButton resBtn = QMessageBox::question( this, "hehe XD", tr( "Are you sure u wanna exit?\n" ), QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes );
	if ( resBtn != QMessageBox::Yes )
	{
		event->ignore();
	}
	else
	{
		event->accept();
	}
}