#pragma once
#include "stdafx.h"
#include "Zdeny_PhD_Shenanigans.h"
#include "Optimization/Evolution.h"
#include "Optimization/OptimizationTestFunctions.h"
#include "Procedural/procedural.h"
#include "Snake/game.h"
#include "Fit/polyfit.h"
#include "Fit/nnfit.h"
#include "IPC/IPC.h"
#include "IPC/IterativePhaseCorrelation.h"
#include "Plot/PlotCSV.h"
#include "Filtering/HistogramEqualization.h"

namespace Debug
{
void Debug(Globals* globals)
{
  TIMER("Debug");
  LOG_STARTEND("Debug started", "Debug finished");

  if (0) // regex test
  {
    std::string str("001:UNTIL=002:NUM=003:USED=004:APP=STT:jjjjjjjj:asdasdasdasdasdIS_KOKOT[=TRUE];IS_PICA[=FOLS];IS_PKOKOTICA[=FKOKOTOLS];");

    std::regex paramsRegex(R"(([0-9]+):UNTIL=([0-9]+):NUM=([0-9]+):USED=([0-9]+):APP=[A-Za-z]+:.{8}:.+)");
    LOG_FATAL("Input string: {}, params regex: {}", str, std::regex_match(str, paramsRegex));
    std::smatch paramsPieces;
    if (std::regex_match(str, paramsPieces, paramsRegex))
      for (size_t i = 0; i < paramsPieces.size(); ++i)
        LOG_FATAL("paramsPieces[{}]: {}", i, paramsPieces[i].str());

    std::regex flagsRegex(R"(.+IS_([A-Za-z]+)\[=([A-Za-z]+)\];)");
    LOG_FATAL("Input string: {}, flags regex: {}", str, std::regex_match(str, flagsRegex));
    std::smatch flagsPieces;
    if (std::regex_match(str, flagsPieces, flagsRegex))
      for (size_t i = 0; i < flagsPieces.size(); ++i)
        LOG_FATAL("flagsPieces[{}]: {}", i, flagsPieces[i].str());

    std::regex e(R"(IS_([A-Za-z]+)\[=([A-Za-z]+)\];)");
    int submatches[] = {1, 2};
    std::regex_token_iterator<std::string::iterator> rend;
    std::regex_token_iterator<std::string::iterator> flagit(str.begin(), str.end(), e, submatches);
    while (flagit != rend)
    {
      std::string flagname = *flagit++;
      std::string flagvalue = *flagit++;
      LOG_INFO("{} / {}", flagname, flagvalue);
    }
  }
  if (0) // rectangular and gaussian abndpass equations
  {
    int rows = 1000;
    int cols = 1000;

    Mat bandpassR = Mat::zeros(rows, cols, CV_32F);
    Mat bandpassG = Mat::zeros(rows, cols, CV_32F);
    Mat gaussL = Mat::zeros(rows, cols, CV_32F);
    Mat gaussH = Mat::zeros(rows, cols, CV_32F);

    float fL = 0.2;
    float fH = 0.8;

    float sL = 1.0;
    float sH = 0.01;

    for (int r = 0; r < rows; ++r)
    {
      for (int c = 0; c < cols; ++c)
      {
        float R = sqrt(std::pow(((float)c - cols / 2) / (cols / 2), 2) + std::pow(((float)r - rows / 2) / (rows / 2), 2));
        bandpassR.at<float>(r, c) = (fL <= R && R <= fH) ? 1 : 0;

        gaussL.at<float>(r, c) = exp(-std::pow(((float)c - cols / 2) / (cols / 2), 2) * std::pow(sL, 2) -
                                     std::pow(((float)r - rows / 2) / (rows / 2), 2) * std::pow(sL, 2));

        gaussH.at<float>(r, c) = 1.0 - exp(-std::pow(((float)c - cols / 2) / (cols / 2), 2) / std::pow(sH, 2) -
                                           std::pow(((float)r - rows / 2) / (rows / 2), 2) / std::pow(sH, 2));

        bandpassG.at<float>(r, c) = gaussL.at<float>(r, c) * gaussH.at<float>(r, c);
      }
    }

    Plot2D::plot(bandpassR, "bandpassR");
    Plot2D::plot(gaussL, "gaussL");
    Plot2D::plot(gaussH, "gaussH");
    Plot2D::plot(bandpassG, "bandpassG");

    // Plot2D::plot(bandpassian(rows, cols, sL, sH), "bandpassian");
  }
  if (0) // cyclic image shift
  {
    Mat img = imread("C:\\Users\\Zdeny\\Downloads\\src.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    img.convertTo(img, CV_32F);

    Mat ref = imread("C:\\Users\\Zdeny\\Downloads\\ref.tif", CV_LOAD_IMAGE_GRAYSCALE);
    ref.convertTo(ref, CV_32F);

    Mat out = img.clone();
    int shiftX = 800;
    int shiftY = 83;

    auto MyModulo = [](int a, int b) { return a - a / b * b >= 0 ? a - a / b * b : a - a / b * b + b; };

    for (int r = 0; r < img.rows; ++r)
    {
      for (int c = 0; c < img.cols; ++c)
      {
        int srcrow = MyModulo(r - shiftY, img.rows);
        int srccol = MyModulo(c - shiftX, img.cols);
        out.at<float>(r, c) = img.at<float>(srcrow, srccol);
      }
    }

    showimg(std::vector<Mat>{img, out, ref}, "cyclic shift");
    Plot2D::plot(abs(ref - out), "diff plot");
    Plot2D::plot(img, "img plot");
  }
  if (0) // histogram equalize
  {
    Mat img = imread("Resources/test.png", CV_LOAD_IMAGE_GRAYSCALE);
    normalize(img, img, 0, 255, CV_MINMAX);
    img.convertTo(img, CV_8UC1);
    resize(img, img, Size(500, 500));
    Mat heq = EqualizeHistogram(img);
    Mat aheq = EqualizeHistogramAdaptive(img, 201);

    ShowHistogram(img, "img histogram");
    ShowHistogram(heq, "heq histogram");
    ShowHistogram(aheq, "aheq histogram");

    showimg(std::vector<Mat>{img, heq, aheq}, "hist eq", false, 0, 1, 1000);
  }
  if (0) // ipc optimize test
  {
    IterativePhaseCorrelation ipc(256, 256);
    ipc.Optimize("Resources/", "Resources/", 0.3, 0.01, 11);
  }
  if (0) // plot from csv file
  {
    PlotCSV::plot("E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\data\\tokens1.csv",
                  "E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\plots\\tokens1.png");
    PlotCSV::plot("E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\data\\tokens2.csv",
                  "E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\plots\\tokens2.png");
  }
  if (0) // plot in optimization
  {
    auto f = OptimizationTestFunctions::Ackley;
    int N = 2;
    Evolution Evo(N);
    Evo.mNP = 50. / 7 * N;
    Evo.mLB = zerovect(N, (double)-N);
    Evo.mUB = zerovect(N, (double)+N);
    Evo.SetParameterNames({"L", "H", "L2", "B", "W", "S"});
    Evo.SetFileOutputDir("E:\\Zdeny_PhD_Shenanigans\\articles\\diffrot\\temp\\");
    Evo.SetOptimizationName("debug opt");
    Evo.SetPlotOutput(true);
    auto result = Evo.Optimize(f);
  }
  if (0) // plot pen colors
  {
    int ncurves = 30;
    int ndata = 100;

    auto x = zerovect(ndata);
    auto ys = zerovect2(ncurves, ndata);

    for (int d = 0; d < ndata; d++)
      x[d] = d;

    for (int c = 0; c < ncurves; c++)
      for (int d = 0; d < ndata; d++)
        ys[c][d] = sin((double)d / ndata * Constants::TwoPi + (double)c / ncurves * Constants::Pi);

    Plot1D::plot(x, ys, "pen colors");
  }
  if (0) // ipc bandpass & window
  {
    IPCsettings set = *globals->IPCset;
    set.setSize(1000, 1000);
    set.setBandpassParameters(5, 1);
    Plot2D::plot(set.bandpass, "x", "y", "z", 0, 1, 0, 1);
  }
  if (0) // 2pic IPC
  {
    std::string path1 = "C:\\Users\\Zdeny\\Documents\\wp\\cat_gray_glance_154511_3840x2160.jpg";
    std::string path2 = "C:\\Users\\Zdeny\\Documents\\wp\\cat_gray_glance_154511_3840x2160.jpg";
    Mat img1 = loadImage(path1);
    Mat img2 = loadImage(path2);

    double shiftX = 10.3;
    double shiftY = -7.2;
    Mat T = (Mat_<float>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
    warpAffine(img2, img2, T, cv::Size(img2.cols, img2.rows));

    IPCsettings set = *globals->IPCset;
    set.speak = IPCsettings::All;
    set.setSize(img1.rows, img1.cols);

    auto shifts = phasecorrel(img1, img2, set);
  }
  if (0) // Plot1D + Plot2D test
  {
    // 1D
    int N = 1000;
    auto X = zerovect(N);
    auto Y1s = zerovect2(3, N, 0.);
    auto Y2s = zerovect2(2, N, 0.);
    double s1 = rand01();
    double s2 = rand01();
    double s3 = rand01();

    for (int x = 0; x < N; x++)
    {
      X[x] = x;

      Y1s[0][x] = s1 * 100.0 * sin(Constants::TwoPi * (double)x / N);
      Y1s[1][x] = s2 * 200.0 * sin(Constants::TwoPi * (double)x / N);
      Y1s[2][x] = s3 * 500.0 * sin(Constants::TwoPi * (double)x / N);

      Y2s[0][x] = s1 * 1.0 * cos(Constants::TwoPi * (double)x / N);
      Y2s[1][x] = s2 * 0.5 * cos(Constants::TwoPi * (double)x / N);
    }

    // 2D
    int Ny = 1000;
    int Nx = 1000;
    auto Z = zerovect2(Ny, Nx, 0.);

    for (int y = 0; y < Ny; y++)
      for (int x = 0; x < Nx; x++)
        Z[y][x] = sin((double)x * y / Nx / Ny * 100) * rand();

    Plot1D::plot(X, Y1s, Y2s, "very nice plot", "X", "Y1", "Y2", std::vector<std::string>{"y1a", "y1b", "y1c"},
                 std::vector<std::string>{"y2a", "y2b"});
    Plot2D::plot(Z, "niceplot", "X", "Y", "Z", 0, 1, 0, 1, 2);
  }
  if (0) // swind crop
  {
    std::string path = "D:\\MainOutput\\S-wind\\";
    int sizeX = 300;
    int sizeY = 200;

    for (int i = 0; i < 10; i++)
    {
      auto pic = imread(path + "0" + to_string(i + 1) + "_calib.PNG", IMREAD_ANYDEPTH);
      pic = roicrop(pic, 0.365 * pic.cols, 0.72 * pic.rows, sizeX, sizeY);
      saveimg(path + "cropped5//crop" + to_string(i) + ".PNG", pic);
    }
  }
  if (0) // 2d poylfit
  {
    int size = 100;
    int size2 = 1000;
    int trials = 500;
    int degree = 5;
    std::vector<double> xdata(trials);
    std::vector<double> ydata(trials);
    std::vector<double> zdata(trials);
    std::vector<Point2f> pts(trials);
    Mat pointiky = Mat::zeros(size2, size2, CV_8UC3);

    Mat orig = Mat::zeros(size, size, CV_32F);
    for (int r = 0; r < size; r++)
    {
      for (int c = 0; c < size; c++)
      {
        double x = (float)c / 99;
        double y = (float)r / 99;
        orig.at<float>(r, c) = sqr(x - 0.75) + sqr(y - 0.25) + 0.1 * sin(0.1 * (x * 99 + y * 99) + 1);
      }
    }

    for (int i = 0; i < trials; i++)
    {
      xdata[i] = rand01();
      ydata[i] = rand01();
      pts[i] = Point2f(xdata[i], ydata[i]);
      zdata[i] = sqr(xdata[i] - 0.75) + sqr(ydata[i] - 0.25) + 0.1 * sin(0.1 * (xdata[i] * 99 + ydata[i] * 99) + 1);
      drawPoint(pointiky, size2 * pts[i], Scalar(rand() % 256, rand() % 256, rand() % 256), 5, 3);
    }

    Mat fitPoly = polyfit(xdata, ydata, zdata, degree, 0, 1, 0, 1, size, size);
    Mat fitNn = nnfit(pts, zdata, 0, 1, 0, 1, size, size);
    Mat fitwNn = wnnfit(pts, zdata, 0, 1, 0, 1, size, size);

    showimg(fitPoly, "fit - polynomial5", true);
    showimg(fitNn, "fit - nearest neighbor", true);
    showimg(fitwNn, "fit - weighted nearest neighbors", true);
    showimg(orig, "original", true);
    showimg(pointiky, "trials");
  }
  if (0) // blaze test
  {
    blaze::DynamicVector<double> a(3);                   // column vec - default
    blaze::DynamicVector<double, blaze::rowVector> b(3); // row vec
    blaze::StaticVector<double, 3> c(1);

    a = {-1, 5, -5};
    b = {1, 2, 3};
    c = {5, 3, 1};

    LOG_DEBUG("a = \n{}", a);
    LOG_DEBUG("b = \n{}", b);
    LOG_DEBUG("c = \n{}", c);
    LOG_DEBUG("a * b = \n{}", a * b);
    LOG_DEBUG("a * b' = \n{}", a * blaze::trans(b));

    blaze::DynamicMatrix<double> A(3, 4);                     // row major - default
    blaze::DynamicMatrix<double, blaze::columnMajor> B(3, 4); // column major
    blaze::StaticMatrix<double, 3, 4> C(1);

    LOG_DEBUG("A = \n{}", A);
    LOG_DEBUG("B = \n{}", B);
    LOG_DEBUG("C = \n{}", C);
    LOG_DEBUG("A + C = \n{}", A + C);
  }
  if (0) // new ipc test
  {
    Mat img1 = loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_00_22__CONT.fits");
    Mat img2 = loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_01_07__CONT.fits");

    int Rows = img1.rows;
    int Cols = img1.cols;
    double BandpassL = 1;
    double BandpassH = 200;
    double L1ratio = 0.35;
    int L2size = 15;
    int UpsampleCoeff = 51;
    double DivisionEpsilon = 0;
    int MaxIterations = 20;
    InterpolationFlags InterpolationType = INTER_LINEAR;
    bool ApplyWindow = true;
    bool ApplyBandpass = true;
    bool SubpixelEstimation = true;
    bool CrossCorrelate = false;

    IPCsettings ipc1(Rows, Cols, BandpassL, BandpassH);
    ipc1.L1ratio = L1ratio;
    ipc1.L2size = L2size;
    ipc1.UC = UpsampleCoeff;
    ipc1.epsilon = DivisionEpsilon;
    ipc1.applyWindow = true;
    ipc1.applyBandpass = true;
    ipc1.subpixel = SubpixelEstimation;
    ipc1.crossCorrel = CrossCorrelate;

    IterativePhaseCorrelation ipc2(Rows, Cols, BandpassL, BandpassH);

    auto shift1 = phasecorrel(img1, img2, ipc1);
    auto shift1n = phasecorrel(img1, img1, ipc1);

    auto shift2 = ipc2.Calculate(img1, img2);
    auto shift2n = ipc2.Calculate(img1, img1);

    LOG_INFO("shift1 = {}", shift1);
    LOG_INFO("shift1n = {}", shift1n);

    LOG_INFO("shift2 = {}", shift2);
    LOG_INFO("shift2n = {}", shift2n);
  }
  if (0) // ipc sign test
  {
    Mat img1 = loadImage("Resources\\test1.png");
    Mat img2 = loadImage("Resources\\test2.png");

    IterativePhaseCorrelation ipc(img1.rows, img1.cols, 0.1, 200);

    auto shift = ipc.Calculate(img1, img2);

    LOG_INFO("shift = {}", shift);
  }
  if (0) // loadfits test
  {
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_00_22__CONT.fits");
    LOG_NEWLINE;
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_01_07__CONT.fits");
    LOG_NEWLINE;
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_19_07__CONT.fits");
    LOG_NEWLINE;
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_19_52__CONT.fits");
    LOG_NEWLINE;
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_02_02__13_16_08__CONT.fits");
  }
  if (0) // loadfits test 2
  {
    LOG_NEWLINE;
    auto pic = loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_02__18_49_52__CONT.fits");
    showimg(pic, "pic");
  }
  if (0) // 1D / 2D sorted xs interp test
  {
    std::vector<double> xs{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<double> ys{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    LOG_INFO("Diffrot interp 1D a) {}", DiffrotResults::Interpolate(xs, ys, -1.0));
    LOG_INFO("Diffrot interp 1D b) {}", DiffrotResults::Interpolate(xs, ys, 10.0));
    LOG_INFO("Diffrot interp 1D c) {}", DiffrotResults::Interpolate(xs, ys, 1));
    LOG_INFO("Diffrot interp 1D d) {}", DiffrotResults::Interpolate(xs, ys, 1.5));

    LOG_NEWLINE;

    auto xsr = xs;
    auto ysr = ys;
    std::reverse(xsr.begin(), xsr.end());
    std::reverse(ysr.begin(), ysr.end());
    LOG_INFO("Diffrot interp 1Dr a) {}", DiffrotResults::Interpolate(xsr, ysr, -1.0));
    LOG_INFO("Diffrot interp 1Dr b) {}", DiffrotResults::Interpolate(xsr, ysr, 10.0));
    LOG_INFO("Diffrot interp 1Dr c) {}", DiffrotResults::Interpolate(xsr, ysr, 1));
    LOG_INFO("Diffrot interp 1Dr d) {}", DiffrotResults::Interpolate(xsr, ysr, 1.5));

    LOG_NEWLINE;

    std::vector<double> xs1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<double> ys1{0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    std::vector<double> xs2{-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<double> ys2{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    std::vector<std::vector<double>> xss = {xs1, xs2};
    std::vector<std::vector<double>> yss = {ys1, ys2};
    LOG_INFO("Diffrot interp 2D a) {}", DiffrotResults::Interpolate(xss, yss, 1.0));
    LOG_INFO("Diffrot interp 2D b) {}", DiffrotResults::Interpolate(xss, yss, -1.0));
    LOG_INFO("Diffrot interp 2D c) {}", DiffrotResults::Interpolate(xss, yss, 1.5));
    LOG_INFO("Diffrot interp 2D d) {}", DiffrotResults::Interpolate(xss, yss, 1.75));
  }
  if (0) // try/catch performance test
  {
    int N = 1e7;
    int iters = 10;
    std::vector<double> vec(N);

    for (int iter = 0; iter < iters; ++iter)
    {
      {
        TIMER("Try/catch -");
        for (auto& x : vec)
          x = sin(rand01());
      }

      {
        TIMER("Try/catch +");
        for (auto& x : vec)
        {
          try
          {
            x = sin(rand01());
          }
          catch (...)
          {
            LOG_FATAL("Exception thrown");
          }
        }
      }
    }
  }
  if (0)
  {
  }
}
}
