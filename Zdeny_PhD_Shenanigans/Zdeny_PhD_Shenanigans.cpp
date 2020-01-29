#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"


Zdeny_PhD_Shenanigans::Zdeny_PhD_Shenanigans(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	//show the logo
	QPixmap pm("logo.png"); // <- path to image file
	ui.label_2->setPixmap(pm);
	ui.label_2->setScaledContents(true);

	//allocate all globals - main window is loaded once only
	globals = new Globals();
	globals->IPCsettings = new IPCsettings(100, 100, 5, 20);
	globals->widget1 = ui.widget;
	globals->widget2 = ui.widget_2;
	globals->plotter2D = new Plotter2D();

	#ifdef LOGGER_QT
	globals->Logger = new QtLogger(g_loglevel, ui.textBrowser);
	#else
	globals->Logger = new CslLogger(g_loglevel);
	#endif

	globals->Logger->Log("Welcome back, my friend.", SPECIAL);//log a welcome message
	//create all windows
	windowIPCparameters = new WindowIPCparameters(this, globals);
	windowIPCoptimize = new WindowIPCoptimize(this, globals);
	windowIPC2PicAlign = new WindowIPC2PicAlign(this, globals);
	windowDiffrot = new WindowDiffrot(this, globals);

	//make signal to slot connections
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(exit()));
	connect(ui.actionAbout_Zdeny_s_PhD_Shenanigans, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.pushButtonAddLogs, SIGNAL(clicked()), this, SLOT(loggerToTextBrowser()));
	connect(ui.pushButtonClearLogs, SIGNAL(clicked()), this, SLOT(clearTextBrowser()));
	connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(closeCV()));
	connect(ui.actionIPC_parameters, SIGNAL(triggered()), this, SLOT(showWindowIPCparameters()));
	connect(ui.actionIPC_optimize, SIGNAL(triggered()), this, SLOT(showWindowIPCoptimize()));
	connect(ui.actionIPC_2pic_align, SIGNAL(triggered()), this, SLOT(showWindowIPC2PicAlign()));
	connect(ui.actionDebug, SIGNAL(triggered()), this, SLOT(debug()));
	connect(ui.actiondiffrot, SIGNAL(triggered()), this, SLOT(showWindowDiffrot()));
}

void Zdeny_PhD_Shenanigans::exit()
{
	QApplication::exit();
}

void Zdeny_PhD_Shenanigans::debug()
{
	if (0)//1D plot
	{
		int n = 10001;
		auto x = zerovect(n);
		auto y = zerovect(n);
		for (int i = 0; i < n; i++)
		{
			x[i] = (double)i/(n-1);
			y[i] = sin(2 * PI*x[i]);
		}

		Plot1D plt(ui.widget);
		plt.plot(x, y);
	}
	if (0)//plot in optimization
	{
		Evolution Evo(2);
		Evo.NP = 10;
		Plot1D plt(ui.widget);
		auto f = [&](std::vector<double> args) 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			return abs(args[0] - args[1]); 
		};
		auto result = Evo.optimize(f, globals->Logger, &plt);
		plt.save("D:\\MainOutput\\Debug\\plot1D.png");
	}
	if (0)//ipc bandpass & window 
	{
		IPCsettings set = *globals->IPCsettings;
		set.setSize(1000, 1000);
		set.setBandpassParameters(5, 1);
		globals->plotter2D->plot(matToVect2(set.bandpass), "x", "y", "z", 0, 1, 0, 1);
		//globals->plotter2D->save("D:\\MainOutput\\Debug\\plot2D.png", 1);
	}
	if (0)//2pic IPC
	{
		std::string path1 = "D:\\MainOutput\\Debug\\test1.PNG";
		std::string path2 = "D:\\MainOutput\\Debug\\test2.PNG";
		Mat img1 = loadImage(path1);
		Mat img2 = loadImage(path2);

		IPCsettings set = *globals->IPCsettings;
		set.IPCshow = true;
		set.setSize(img1.rows, img1.cols);

		auto shifts = phasecorrel(img1, img2, set, globals->Logger, globals->plotter2D);
	}
	if (1)//inverse mapping
	{
		#include "inverseMapping.h"
		int trials = 50;
		int N = 4;

		std::vector<std::vector<double>> trialInputs;
		std::vector<std::vector<double>> trialOutputs;
		std::vector<double> desiredOutput;

		for (int trial = 0; trial < trials; trial++)
		{
			std::vector<double> trialInput;
			for (int x = 0; x < N; x++)
				trialInput.push_back((double)rand() / RAND_MAX);
			trialInputs.push_back(trialInput);
			trialOutputs.push_back(inverseMappingTestTransfer(trialInput));
		}

		for (int x = 0; x < N; x++)
			desiredOutput.push_back(x + 1);//123456789

		std::vector<double> soughtInput = inverseMappingZH(trialInputs, trialOutputs, desiredOutput);
		std::vector<double> realOutput = inverseMappingTestTransfer(soughtInput);

		cout << "trialInputs: " << trialInputs << endl;
		cout << "trialOutputs: " << trialOutputs << endl;
		cout << endl;
		cout << "soughtInput: " << soughtInput << endl;
		cout << "desiredOutput: " << desiredOutput << endl;
		cout << "realOutput: " << realOutput << endl;
	}

	globals->Logger->Log("Debug finished.", EVENT);
}

void Zdeny_PhD_Shenanigans::about()
{
	QMessageBox msgBox;
	msgBox.setText("All these shenanigans were created during my PhD studies.\n\nHave fun,\nZdenek Hrazdira\n2018-2020");
	msgBox.exec();
}

void Zdeny_PhD_Shenanigans::closeCV()
{
	destroyAllWindows();
	globals->Logger->Log("All image windows closed", INFO);
}

void Zdeny_PhD_Shenanigans::loggerToTextBrowser()
{
	for (int i = 0; i < 100; i++)
	{
		globals->Logger->Log("boziiinku, co ted budeme delat?", FATAL);
		globals->Logger->Log("jejda, nebude v tom nahodou nejaky probljhbemek?", EVENT);
		globals->Logger->Log("trololooo niga xdxd 5", SUBEVENT);
		globals->Logger->Log("objednal jsem ti ten novy monitor, jak jsi chtel", INFO);
		globals->Logger->Log("tak tenhle zapeklity problemek budeme muset", DEBUG);
	}
}

void Zdeny_PhD_Shenanigans::clearTextBrowser()
{
	ui.textBrowser->clear();
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
	windowIPCparameters->show();
	windowDiffrot->show();
}
