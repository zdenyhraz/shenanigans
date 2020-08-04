#include "stdafx.h"
#include "Gui/Main/Zdeny_PhD_Shenanigans.h"

#ifdef ATTACH_CONSOLE
	#include <windows.h>
	#include <stdio.h>
#endif

int main( int argc, char *argv[] )
{
#ifdef ATTACH_CONSOLE
	// detach from the current console window
	// if launched from a console window, that will still run waiting for the new console (below) to close
	// it is useful to detach from Qt Creator's <Application output> panel
	FreeConsole();

	// create a separate new console window
	AllocConsole();

	// attach the new console to this application's process
	AttachConsole( GetCurrentProcessId() );

	HWND console = GetConsoleWindow();
	int consolewidth = 1500;
	int consoleheight = 500;
	MoveWindow( console, 0, 1400 - consoleheight, consolewidth, consoleheight, TRUE );

	// reopen the std I/O streams to redirect I/O to the new console
	freopen( "CON", "w", stdout );
	freopen( "CON", "w", stderr );
	freopen( "CON", "r", stdin );
#endif

	QApplication::setAttribute( Qt::AA_EnableHighDpiScaling ); // DPI support
	QCoreApplication::setAttribute( Qt::AA_UseHighDpiPixmaps ); //HiDPI pixmaps
	//qputenv("QT_SCALE_FACTOR", "1.0");

	QApplication a( argc, argv );
	Zdeny_PhD_Shenanigans w;
	w.show();
	return a.exec();
}
