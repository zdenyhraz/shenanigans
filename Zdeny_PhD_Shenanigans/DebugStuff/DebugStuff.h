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
#include "Sasko/NonMaximaSuppression.h"
#include "Log/QtLogger.h"
#include "Core/BlazeUtils.h"
#include "Complexity/ComplexityClassEstimation.h"

namespace Debug
{
void Debug(Globals* globals)
{
  LOG_FUNCTION("Debug");

  if (1) // complexity estimation test
  {
    const auto f = [](const std::vector<double>& x) {
      double g = 0;

      for (int i = 0; i < 1e7; ++i) // constant portion
        g += i;

      const int scale = 1;
      for (int i = 0; i < scale; ++i) // scale
        for (const auto& a : x)       // n^2
          for (const auto& b : x)
            g += a - b;

      return g;
    };
    EstimateComplexity(f);
    return;
  }
  if (0) // sasko DFT test
  {
    Mat img1 = roicropmid(loadImage("Resources/test.png"), 1000, 1000);
    Mat img2;
    flip(img1, img2, 1);
    rotate(img2, img2, ROTATE_90_COUNTERCLOCKWISE);
    Plot2D::Plot("img1", img1);
    Plot2D::Plot("img2", img2);
    Mat fft1 = Fourier::fft(img1);
    Mat fft2 = Fourier::fft(img2);

    transpose(fft2, fft2);

    Fourier::fftshift(fft1);
    Fourier::fftshift(fft2);
    Plot2D::Plot("fft1", Fourier::logmagn(fft1));
    Plot2D::Plot("fft2", Fourier::logmagn(fft2));
    Plot2D::Plot("fft diff", abs(Fourier::logmagn(fft2) - Fourier::logmagn(fft1)));

    Mat planes1[2], planes2[2];
    split(fft1, planes1);
    split(fft2, planes2);
    log(planes1[0], planes1[0]);
    log(planes1[1], planes1[1]);
    log(planes2[0], planes2[0]);
    log(planes2[1], planes2[1]);

    Plot2D::Plot("re diff", abs(planes1[0] - planes2[0]));
    Plot2D::Plot("im diff", abs(planes1[1] - planes2[1]));
  }
  if (0) // ipc shift test
  {
    Mat img1 = loadImage("../articles/swind/source/1/cropped/crop1.png");
    Mat img2 = loadImage("../articles/swind/source/1/cropped/crop2.png");

    int sajz = 0;
    Size size(sajz, sajz);
    if (sajz > 0 && img1.size() != size)
    {
      resize(img1, img1, size);
      resize(img2, img2, size);
    }

    bool crop = true;
    if (crop)
    {
      int cropsize = 128;
      img1 = roicrop(img1, img1.cols * 0.6, img1.rows * 0.63, cropsize, cropsize);
      img2 = roicrop(img2, img2.cols * 0.6, img2.rows * 0.63, cropsize, cropsize);
    }

    IterativePhaseCorrelation ipc = *globals->IPC;
    ipc.SetSize(img1.rows, img1.cols);
    ipc.SetDebugMode(true);
    // ipc.SetWindowType(IterativePhaseCorrelation::WindowType::Hann);
    // ipc.SetBandpassType(IterativePhaseCorrelation::BandpassType::Gaussian);

    const auto shiftCalc = ipc.Calculate(img1, img2);
    LOG_DEBUG("IPC shift: [{}]", shiftCalc);
    return;
  }
  if (0) // opencv fft with blaze complex matrix memory layout
  {
    const auto path = "Resources/171A.png";
    const auto imgCV = loadImage(path);
    const auto imgBlaze = LoadImageBlaze(path);

    Mat fftCV;
    BlazeMatComplex fftBlaze;

    showimg(imgCV, "image CV");
    showimg(WrapOpenCVMat(imgBlaze), "image Blaze");

    {
      LOG_FUNCTION("CV fft");
      Mat planes[] = {imgCV, Mat::zeros(imgCV.size(), CV_32F)};
      merge(planes, 2, fftCV);
      dft(fftCV, fftCV, DFT_COMPLEX_OUTPUT);
    }

    {
      LOG_FUNCTION("Blaze fft");
      fftBlaze = blaze::map(imgBlaze, [](float real) { return std::complex<float>(real, 0); });
      dft(WrapOpenCVMat(fftBlaze), WrapOpenCVMat(fftBlaze), DFT_COMPLEX_OUTPUT);
    }

    showfourier(fftCV, true, false, "FFT CV");
    showfourier(WrapOpenCVMat(fftBlaze), true, false, "FFT BLAZE");

    {
      LOG_FUNCTION("CV ifft");
      dft(fftCV, fftCV, DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);
    }

    {
      LOG_FUNCTION("Blaze ifft");
      dft(WrapOpenCVMat(fftBlaze), WrapOpenCVMatReal(fftBlaze), DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);
    }

    showimg(fftCV, "image CV out");
    showimg(WrapOpenCVMatReal(fftBlaze), "image Blaze out");
  }
  if (0) // opencv fft with blaze benchmark
  {
    const auto path = "Resources/snake.png";
    auto imgOpenCV = loadImage(path);
    auto imgBlaze = WrapOpenCVMat(LoadImageBlaze(path)).clone();
    Mat outCV, outBlaze;

    // pre-alloc
    outCV = Fourier::fft(imgOpenCV);
    outBlaze = Fourier::fft(imgBlaze);

    {
      LOG_FUNCTION("OpenCV fft");
      outCV = Fourier::fft(imgOpenCV);
    }

    {
      LOG_FUNCTION("Blaze-OpenCV fft");
      outBlaze = Fourier::fft(imgBlaze);
    }

    {
      LOG_FUNCTION("OpenCV fft move");
      outCV = Fourier::fft(std::move(imgOpenCV));
    }

    {
      LOG_FUNCTION("Blaze-OpenCV fft move");
      outBlaze = Fourier::fft(std::move(imgBlaze));
    }

    showfourier(outCV, true, false, "FFT CV");
    showfourier(outBlaze, true, false, "FFT BLAZE-CV");
  }
  if (0) // blaze matmul vs opencv matmul benchmark
  {
    const auto iters = 1000;
    const auto path = "Resources/171A.png";
    const auto imgOpenCV = loadImage(path);
    const auto imgBlaze = LoadImageBlaze(path);
    auto imgOpenCVOut = imgOpenCV.clone();
    auto imgBlazeOut = imgBlaze;

    LOG_INFO("Matrix size: {}", imgOpenCV.size());

    {
      LOG_FUNCTION("OpenCV matmul");
      for (int i = 0; i < iters; ++i)
        imgOpenCVOut = imgOpenCV.mul(imgOpenCV);
    }

    {
      LOG_FUNCTION("Blaze matmul");
      for (int i = 0; i < iters; ++i)
        imgBlazeOut = imgBlaze % imgBlaze;
    }

    LOG_DEBUG("One element equality test: {} & {}", imgOpenCVOut.at<float>(imgOpenCV.rows / 2, imgOpenCV.cols / 2), imgBlazeOut(imgOpenCV.rows / 2, imgOpenCV.cols / 2));
  }
  if (0) // swind crop
  {
    std::string path = "../articles/swind/source/2";
    double scale = 0.8;
    int sizeX = 1500;
    int sizeY = 1000;

    for (int i = 0; i < 11; i++)
    {
      auto pic = imread(fmt::format("{}/raw/0{}_calib.PNG", path, i + 1), IMREAD_ANYDEPTH);
      pic = roicrop(pic, 0.33 * pic.cols, 0.69 * pic.rows, sizeX, sizeY);
      saveimg(fmt::format("{}/cropped/crop{}.PNG", path, i), pic);
    }
  }
  if (0) // ipc align test
  {
    // silocary @1675/2200

    Point cropfocus(2048, 2048);
    int cropsize = 1.0 * 4096;

    Mat img1 = roicrop(loadImage("Resources/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);
    Mat img2 = roicrop(loadImage("Resources/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);

    int size = cropsize;
    resize(img1, img1, Size(size, size));
    resize(img2, img2, Size(size, size));

    Shift(img2, -950, 1050);
    Rotate(img2, 70, 1.2);

    IterativePhaseCorrelation ipc = *globals->IPC;
    ipc.SetSize(img1.size());
    ipc.SetDebugMode(true);
    Mat aligned = ipc.Align(img1, img2);
    showimg(std::vector<Mat>{img1, img2, aligned}, "align triplet");
  }
  if (0) // dft vs cuda::dft
  {
    const int minsize = 16;
    const int maxsize = 512;
    const int sizeStep = 10;
    const int iters = (maxsize - minsize + 1) / sizeStep % 2 ? (maxsize - minsize + 1) / sizeStep : (maxsize - minsize + 1) / sizeStep + 1;
    const int itersPerSize = 100;
    Mat img = loadImage("Resources/test.png");
    std::vector<double> sizes(iters);
    std::vector<double> timeCpu(iters);
    std::vector<double> timeGpu(iters);
    Mat fft = Fourier::cufft(img); // init

    for (int i = 0; i < iters; ++i)
    {
      int size = minsize + (float)i / (iters - 1) * (maxsize - minsize);
      sizes[i] = size;
      LOG_INFO("{} / {} Calculating FFT speeds for image size {}", i + 1, iters, size);
      Mat resized = roicrop(img, img.cols / 2, img.rows / 2, size, size);

      {
        auto start = std::chrono::high_resolution_clock::now();

        for (int it = 0; it < itersPerSize; ++it)
          Mat fft = Fourier::fft(resized);

        auto end = std::chrono::high_resolution_clock::now();
        timeCpu[i] = (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / itersPerSize;
      }

      {
        auto start = std::chrono::high_resolution_clock::now();

        for (int it = 0; it < itersPerSize; ++it)
          Mat fft = Fourier::cufft(resized);

        auto end = std::chrono::high_resolution_clock::now();
        timeGpu[i] = (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / itersPerSize;
      }
    }

    Plot1D::Reset("FFT CPU vs GPU");
    Plot1D::SetYnames({"fft cpu", "fft gpu"});
    Plot1D::SetYlabel("time per DFT [ms]");
    Plot1D::SetXlabel("image size");
    Plot1D::Plot("FFT CPU vs GPU", sizes, {timeCpu, timeGpu}, false);
    return;
  }
  if (0) // cuda
  {
    Mat img = loadImage("Resources/test.png");
    Mat fft, fftpacked, cufft, cufftpacked;
    Mat TB, TBpacked, cuTB, cuTBpacked;
    bool cpu = 1;
    bool gpu = 0;

    if (cpu)
    {
      LOG_FUNCTION("fft");
      fft = Fourier::fft(img);
      TB = Fourier::ifft(fft);
    }
    if (cpu)
    {
      LOG_FUNCTION("fftpacked");
      fftpacked = Fourier::fft(img, true);
      TBpacked = Fourier::ifft(fftpacked, true);
    }
    if (gpu)
    {
      LOG_FUNCTION("cufft");
      cufft = Fourier::cufft(img);
      cuTB = Fourier::icufft(cufft);
    }
    if (gpu)
    {
      LOG_FUNCTION("cufftpacked");
      cufftpacked = Fourier::cufft(img, true);
      cuTBpacked = Fourier::icufft(cufftpacked, true);
    }
    if (cpu)
    {
      Plot2D::Plot("img", img);
      Plot2D::Plot("fft logmagn", Fourier::logmagn(fft));
      Plot2D::Plot("TB", TB);

      Plot2D::Plot("img", img);
      Plot2D::Plot("fftpacked logmagn", Fourier::logmagn(fftpacked));
      Plot2D::Plot("TBpacked", TBpacked);
    }
    if (gpu)
    {
      Plot2D::Plot("cuimg", img);
      Plot2D::Plot("cufft logmagn", Fourier::logmagn(cufft));
      Plot2D::Plot("cuTB", cuTB);

      Plot2D::Plot("cuimgsym", img);
      Plot2D::Plot("cufftpacked logmagn", Fourier::logmagn(cufftpacked));
      Plot2D::Plot("cuTBpacked", cuTBpacked);
    }

    // Plot2D::Plot("cufft-fft logmagn absdiff", abs(Fourier::logmagn(cufft) - Fourier::logmagn(fft)));
    // Plot2D::Plot("cuTB-TB absdiff", abs(cuTB - TB));
  }
  if (0) // log levels
  {
    LOG_TRACE("Trace boiiii xdxd {} a {} a {}", 1, 2, 3);
    LOG_DEBUG("Debug boiiii xdxd {} a {} a {}", 1, 2, 3);
    LOG_INFO("Info boiiii xdxd {} a {} a {}", 1, 2, 3);
    LOG_SUCCESS("Success boiiii xdxd {} a {} a {}", 1, 2, 3);
    LOG_WARNING("Warning boiiii xdxd {} a {} a {}", 1, 2, 3);
    LOG_ERROR("Error boiiii xdxd {} a {} a {}", 1, 2, 3);
  }
  if (0) // OpenCV pdb test
  {
    Mat mat(100, 100, CV_32F);
    roicrop(mat, 4, 4, 11, 11);
    return;
  }
  if (0) // non maxima suppression
  {
    Mat img = loadImage("Resources/test.png");
    float scale = 1.0;
    resize(img, img, Size(scale * img.cols, scale * img.rows));
    NonMaximaSuppresion(img);
  }
  if (0) // kirkl test
  {
    for (int i = 5; i < 21; i += 2)
    {
      Mat kirklik;
      resize(kirkl(i), kirklik, Size(999, 999), 0, 0, INTER_NEAREST);
      // Plot2D::Plot(kirklik, std::string("kirkl") + i);
    }
    // Plot2D::Plot(kirkl(999), std::string("kirkl999"));
  }
  if (0) // regex test
  {
    std::string str("001:UNTIL=002:NUM=003:USED=004:APP=STT:jjjjjjjj:asdasdasdasdasdIS_KOKOT[=TRUE];IS_PICA[=FOLS];IS_PKOKOTICA[=FKOKOTOLS];");

    std::regex paramsRegex(R"(([0-9]+):UNTIL=([0-9]+):NUM=([0-9]+):USED=([0-9]+):APP=[A-Za-z]+:.{8}:.+)");
    LOG_ERROR("Input string: {}, params regex: {}", str, std::regex_match(str, paramsRegex));
    std::smatch paramsPieces;
    if (std::regex_match(str, paramsPieces, paramsRegex))
      for (size_t i = 0; i < paramsPieces.size(); ++i)
        LOG_ERROR("paramsPieces[{}]: {}", i, paramsPieces[i].str());

    std::regex flagsRegex(R"(.+IS_([A-Za-z]+)\[=([A-Za-z]+)\];)");
    LOG_ERROR("Input string: {}, flags regex: {}", str, std::regex_match(str, flagsRegex));
    std::smatch flagsPieces;
    if (std::regex_match(str, flagsPieces, flagsRegex))
      for (size_t i = 0; i < flagsPieces.size(); ++i)
        LOG_ERROR("flagsPieces[{}]: {}", i, flagsPieces[i].str());

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

        gaussL.at<float>(r, c) = exp(-std::pow(((float)c - cols / 2) / (cols / 2), 2) * std::pow(sL, 2) - std::pow(((float)r - rows / 2) / (rows / 2), 2) * std::pow(sL, 2));

        gaussH.at<float>(r, c) = 1.0 - exp(-std::pow(((float)c - cols / 2) / (cols / 2), 2) / std::pow(sH, 2) - std::pow(((float)r - rows / 2) / (rows / 2), 2) / std::pow(sH, 2));

        bandpassG.at<float>(r, c) = gaussL.at<float>(r, c) * gaussH.at<float>(r, c);
      }
    }

    // Plot2D::Plot(bandpassR, "bandpassR");
    // Plot2D::Plot(gaussL, "gaussL");
    // Plot2D::Plot(gaussH, "gaussH");
    // Plot2D::Plot(bandpassG, "bandpassG");

    // Plot2D::Plot(bandpassian(rows, cols, sL, sH), "bandpassian");
  }
  if (0) // cyclic image shift
  {
    Mat img = imread("C:\\Users\\Zdeny\\Downloads\\src.jpg", IMREAD_GRAYSCALE);
    img.convertTo(img, CV_32F);

    Mat ref = imread("C:\\Users\\Zdeny\\Downloads\\ref.tif", IMREAD_GRAYSCALE);
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
    // Plot2D::Plot(abs(ref - out), "diff plot");
    // Plot2D::Plot(img, "img plot");
  }
  if (0) // histogram equalize
  {
    Mat img = imread("Resources/test.png", IMREAD_GRAYSCALE);
    normalize(img, img, 0, 255, NORM_MINMAX);
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
    PlotCSV::plot("E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\data\\tokens1.csv", "E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\plots\\tokens1.png");
    PlotCSV::plot("E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\data\\tokens2.csv", "E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\plots\\tokens2.png");
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

    // Plot1D::Plot(x, ys, "pen colors");
  }
  if (0) // ipc bandpass & window
  {
    IPCsettings set = *globals->IPCset;
    set.setSize(1000, 1000);
    set.setBandpassParameters(5, 1);
    // Plot2D::Plot(set.bandpass, "x", "y", "z", 0, 1, 0, 1);
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

    // Plot1D::Plot(X, Y1s, Y2s, "very nice plot", "X", "Y1", "Y2", std::vector<std::string>{"y1a", "y1b", "y1c"}, std::vector<std::string>{"y2a", "y2b"});
    // Plot2D::Plot(Z, "niceplot", "X", "Y", "Z", 0, 1, 0, 1, 2);
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
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_01_07__CONT.fits");
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_19_07__CONT.fits");
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_19_52__CONT.fits");
    loadImage("D:\\SDOpics\\Calm2020stride25\\2020_02_02__13_16_08__CONT.fits");
  }
  if (0) // loadfits test 2
  {
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

    auto xsr = xs;
    auto ysr = ys;
    std::reverse(xsr.begin(), xsr.end());
    std::reverse(ysr.begin(), ysr.end());
    LOG_INFO("Diffrot interp 1Dr a) {}", DiffrotResults::Interpolate(xsr, ysr, -1.0));
    LOG_INFO("Diffrot interp 1Dr b) {}", DiffrotResults::Interpolate(xsr, ysr, 10.0));
    LOG_INFO("Diffrot interp 1Dr c) {}", DiffrotResults::Interpolate(xsr, ysr, 1));
    LOG_INFO("Diffrot interp 1Dr d) {}", DiffrotResults::Interpolate(xsr, ysr, 1.5));

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
            LOG_ERROR("Exception thrown");
          }
        }
      }
    }
  }

  // plot in optimization - default
  auto f = OptimizationTestFunctions::Paraboloid;
  int N = 3;
  Evolution Evo(N);
  Evo.mNP = 10 * N;
  Evo.mLB = zerovect(N, (double)-N);
  Evo.mUB = zerovect(N, (double)+N);
  Evo.SetParameterNames({"L", "H", "L2", "B", "W", "S"});
  Evo.SetFileOutputDir("E:\\Zdeny_PhD_Shenanigans\\articles\\diffrot\\temp\\");
  Evo.SetOptimizationName("debug opt");
  Evo.SetPlotOutput(true);
  auto result = Evo.Optimize(f);
}
}
