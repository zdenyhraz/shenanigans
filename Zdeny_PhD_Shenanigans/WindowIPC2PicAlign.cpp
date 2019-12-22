#include "stdafx.h"
#include "WindowIPC2PicAlign.h"
#include "functionsAstro.h"
#include "plotterqt.h"

WindowIPC2PicAlign::WindowIPC2PicAlign(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
	ui.setupUi(this);
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(align()));
	connect(ui.pushButton_4, SIGNAL(clicked()), this, SLOT(alignXY()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(flowMap()));
	connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(features()));
	connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(linearFlow()));
}

void WindowIPC2PicAlign::align()
{
	std::string path1 = ui.lineEdit->text().toStdString();
	std::string path2 = ui.lineEdit_2->text().toStdString();
	Mat img1 = loadImage(path1);
	Mat img2 = loadImage(path2);

	int size = globals->IPCsettings->getcols();
	img1 = roicrop(img1, 0.375*img1.cols, 0.7*img1.rows, size, size);
	img2 = roicrop(img2, 0.375*img2.cols, 0.7*img2.rows, size, size);

	IPCsettings set = *globals->IPCsettings;
	set.IPCshow = true;

	if (0)//artificial misalign
	{
		img2 = img1.clone();
		double angle = 0;
		double scale = 1;
		double shiftX = 0;
		double shiftY = 0;
		Point2f center((float)img1.cols / 2, (float)img1.rows / 2);
		cout << "Artificial parameters:" << endl << "Angle: " << angle << endl << "Scale: " << scale << endl << "ShiftX: " << shiftX << endl << "ShiftY: " << shiftY << endl;
		Mat R = getRotationMatrix2D(center, angle, scale);
		Mat T = (Mat_<float>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
		warpAffine(img2, img2, T, cv::Size(img1.cols, img1.rows));
		warpAffine(img2, img2, R, cv::Size(img1.cols, img1.rows));
	}

	showimg(img1, "img1");
	showimg(img2, "img2");
	showimg(AlignStereovision(img1, img2), "img 1n2 not aligned");
	alignPics(img1, img2, img2, set);
	showimg(AlignStereovision(img1, img2), "img 1n2 yes aligned");
}

void WindowIPC2PicAlign::alignXY()
{
	globals->Logger->Log("Starting IPC image align process (XY)", EVENT);
	std::string path1 = ui.lineEdit->text().toStdString();
	std::string path2 = ui.lineEdit_2->text().toStdString();
	Mat img1 = loadImage(path1);
	Mat img2 = loadImage(path2);

	int size = globals->IPCsettings->getcols();
	img1 = roicrop(img1, 0.38*img1.cols, 0.74*img1.rows, size, size);
	img2 = roicrop(img2, 0.38*img2.cols, 0.74*img2.rows, size, size);

	IPCsettings set = *globals->IPCsettings;
	set.IPCshow = true;
	set.setSize(img1.rows, img1.cols);
	Plot2D plt(globals->widget, "pixel x", "pixel y", "r");

	auto shifts = phasecorrel(img1, img2, set, globals->Logger);
	globals->Logger->Log("Images aligned & IPC process shown (XY)", EVENT);
}

void WindowIPC2PicAlign::flowMap()
{
	std::string path1 = ui.lineEdit->text().toStdString();
	std::string path2 = ui.lineEdit_2->text().toStdString();
	Mat img1 = loadImage(path1);
	Mat img2 = loadImage(path2);

	int size = 1024;
	img1 = roicrop(img1, 0.4*img1.cols, 0.7*img1.rows, size, size);
	img2 = roicrop(img2, 0.4*img2.cols, 0.7*img2.rows, size, size);

	showimg(img1, "img");
	Mat flowAD = abs(img1 - img2);
	showimg(flowAD, "flowAD", true, 0, 1);

	auto flowMap = calculateFlowMap(img1, img2, *globals->IPCsettings, 1);
	Mat flowX = std::get<0>(flowMap);
	Mat flowY = std::get<1>(flowMap);

	Mat flowM, flowP;
	magnitude(flowX, flowY, flowM);
	phase(flowX, flowY, flowP);

	showimg(flowX, "flowX", true);
	showimg(flowY, "flowY", true);
	showimg(flowM, "flowM", true);
	showimg(flowP, "flowP", true);
}

void WindowIPC2PicAlign::features()
{
	using namespace cv;
	std::string path1 = ui.lineEdit->text().toStdString();
	std::string path2 = ui.lineEdit_2->text().toStdString();
	Mat img1 = loadImage(path1);
	Mat img2 = loadImage(path2);
}

void WindowIPC2PicAlign::linearFlow()
{
	auto xyi = calculateLinearSwindFlow(*globals->IPCsettings, ui.lineEdit_3->text().toStdString());
	auto xshifts = std::get<0>(xyi);
	auto yshifts = std::get<1>(xyi);
	auto indices = std::get<2>(xyi);
	Plot1D plt(globals->widget, "picture index", "pixel shift X", "pixel shift Y");
	plt.plot(indices, xshifts, yshifts);

	globals->Logger->Log("xshifts min = " + to_string(vectorMin(xshifts)), INFO);
	globals->Logger->Log("xshifts max = " + to_string(vectorMax(xshifts)), INFO);
	globals->Logger->Log("yshifts min = " + to_string(vectorMin(yshifts)), INFO);
	globals->Logger->Log("yshifts max = " + to_string(vectorMax(yshifts)), INFO);
	globals->Logger->Log("Linear solar wind speed calculated", EVENT);
}
