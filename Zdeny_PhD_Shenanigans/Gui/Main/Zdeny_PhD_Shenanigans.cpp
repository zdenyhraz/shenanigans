#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"
#include "Optimization/optimization.h"
#include "Procedural/procedural.h"
#include "Snake/game.h"

Zdeny_PhD_Shenanigans::Zdeny_PhD_Shenanigans( QWidget *parent ) : QMainWindow( parent )
{
	ui.setupUi( this );

	//show the logo
	QPixmap pm( "Resources/logo.png" ); // <- path to image file
	ui.label_2->setPixmap( pm );
	ui.label_2->setScaledContents( true );

	//allocate all globals - main window is loaded once only
	globals = new Globals();

	//create all windows
	windowIPCparameters = new WindowIPCparameters( this, globals );
	windowIPCoptimize = new WindowIPCoptimize( this, globals );
	windowIPC2PicAlign = new WindowIPC2PicAlign( this, globals );
	windowDiffrot = new WindowDiffrot( this, globals );
	windowFeatures = new WindowFeatures( this, globals );

	//make signal to slot connections
	connect( ui.actionExit, SIGNAL( triggered() ), this, SLOT( exit() ) );
	connect( ui.actionAbout_Zdeny_s_PhD_Shenanigans, SIGNAL( triggered() ), this, SLOT( about() ) );
	connect( ui.pushButtonClose, SIGNAL( clicked() ), this, SLOT( CloseAll() ) );
	connect( ui.actionIPC_parameters, SIGNAL( triggered() ), this, SLOT( showWindowIPCparameters() ) );
	connect( ui.actionIPC_optimize, SIGNAL( triggered() ), this, SLOT( showWindowIPCoptimize() ) );
	connect( ui.actionIPC_2pic_align, SIGNAL( triggered() ), this, SLOT( showWindowIPC2PicAlign() ) );
	connect( ui.actionDebug, SIGNAL( triggered() ), this, SLOT( debug() ) );
	connect( ui.pushButtonDebug, SIGNAL( clicked() ), this, SLOT( debug() ) );
	connect( ui.actiondiffrot, SIGNAL( triggered() ), this, SLOT( showWindowDiffrot() ) );
	connect( ui.actionPlay, SIGNAL( triggered() ), this, SLOT( playSnake() ) );
	connect( ui.actionGenerate_land, SIGNAL( triggered() ), this, SLOT( generateLand() ) );
	connect( ui.actionFits_downloader, SIGNAL( triggered() ), this, SLOT( fitsDownloader() ) );
	connect( ui.actionFits_checker, SIGNAL( triggered() ), this, SLOT( fitsDownloadChecker() ) );
	connect( ui.actionFeature_match, SIGNAL( triggered() ), this, SLOT( featureMatch() ) );

	LOG_SUCC( "Welcome back, my friend." ); //log a welcome message
}

void Zdeny_PhD_Shenanigans::exit()
{
	QApplication::exit();
}

void Zdeny_PhD_Shenanigans::debug()
{
	if ( 0 ) //plot in optimization
	{
		Evolution Evo( 2 );
		Evo.NP = 10;
		auto f = [&]( std::vector<double> args )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
			return abs( args[0] - args[1] );
		};
		auto result = Evo.optimize( f );
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
	if ( 0 ) //Plot1D + Plot2D test
	{
		// 1D
		int N = 1000;
		auto X = zerovect( N );
		auto Y1s = zerovect2( 3, N );
		auto Y2s = zerovect2( 2, N );
		double s1 = randunit();
		double s2 = randunit();
		double s3 = randunit();

		for ( int x = 0; x < N; x++ )
		{
			X[x] = x;

			Y1s[0][x] = s1 * 100.0 * sin( Constants::TwoPi * ( double )x / N );
			Y1s[1][x] = s2 * 200.0 * sin( Constants::TwoPi * ( double )x / N );
			Y1s[2][x] = s3 * 500.0 * sin( Constants::TwoPi * ( double )x / N );

			Y2s[0][x] = s1 * 1.0 * cos( Constants::TwoPi * ( double )x / N );
			Y2s[1][x] = s2 * 0.5 * cos( Constants::TwoPi * ( double )x / N );
		}

		// 2D
		int Ny = 1000;
		int Nx = 1000;
		auto Z = zerovect2( Ny, Nx );

		for ( int y = 0; y < Ny; y++ )
		{
			for ( int x = 0; x < Nx; x++ )
			{
				Z[y][x] = rand();
			}
		}

		Plot1D::plot( X, Y1s, Y2s, "very nice plot", "X", "Y1", "Y2", std::vector<std::string> {"y1a", "y1b", "y1c"}, std::vector<std::string> {"y2a", "y2b"} );
		Plot2D::plot( Z, "niceplot", "X", "Y", "Z", 0, 1, 0, 1, 2 );
	}
	if ( 0 )// swind crop
	{
		std::string path = "D:\\MainOutput\\S-wind\\";
		int sizeX = 300;
		int sizeY = 200;

		for ( int i = 0; i < 10; i++ )
		{
			auto pic = imread( path + "0" + to_string( i + 1 ) + "_calib.PNG", IMREAD_ANYDEPTH );
			pic = roicrop( pic, 0.365 * pic.cols, 0.72 * pic.rows, sizeX, sizeY );
			saveimg( path + "cropped5//crop" + to_string( i ) + ".PNG", pic );
		}
	}
	if ( 1 ) //2d poylfit
	{
		int trials = 100;
		int degree = 5;
		std::vector<double> xdata( trials );
		std::vector<double> ydata( trials );
		std::vector<double> zdata( trials );

		for ( int i = 0; i < trials; i++ )
		{
			xdata[i] = randunit();
			ydata[i] = randunit();
			zdata[i] = sqr( xdata[i] - 0.75 ) + sqr( ydata[i] - 0.25 );
		}


		Mat pic = polyfit( xdata, ydata, zdata, degree, 0, 1, 0, 1, 100, 100 );
		showimg( pic, "polyfit2d", true );
	}

	LOG_INFO( "Debug finished." );
}

void Zdeny_PhD_Shenanigans::about()
{
	QMessageBox msgBox;
	msgBox.setText( "All these shenanigans were created during my PhD studies.\n\nHave fun,\nZdenek Hrazdira\n2018-2020" );
	msgBox.exec();
}

void Zdeny_PhD_Shenanigans::CloseAll()
{
	destroyAllWindows();
	Plot1D::CloseAll();
	Plot2D::CloseAll();
	LOG_INFO( "All image & plot windows closed" );
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
	std::string urlmain = "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122";
	//generateFitsDownloadUrlSingles( 1, 2000, urlmain );
	generateFitsDownloadUrlPairs( 1, 25, 2000, urlmain );
	LOG_INFO( "Fits download urls created" );
}

void Zdeny_PhD_Shenanigans::fitsDownloadChecker()
{
	std::string urlmain = "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122";
	std::string path = "D:\\SDOpics\\Calm2020stride\\";
	checkFitsDownloadUrlPairs( 1, 25, 2000, urlmain, path );
	LOG_INFO( "Fits download urls checked" );
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
		CloseAll();
	}
}

void Zdeny_PhD_Shenanigans::featureMatch()
{
	windowFeatures->show();
}