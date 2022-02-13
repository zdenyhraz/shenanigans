
#include <regex>
#include "WindowShenanigans.h"
#include "../IPC/WindowIPC.h"
#include "../Diffrot/WindowDiffrot.h"
#include "../Features/WindowFeatures.h"
#include "../FITS/WindowFITS.h"
#include "../Filtering/WindowFiltering.h"
#include "Optimization/Evolution.h"
#include "Optimization/PatternSearch.h"
#include "Optimization/OptimizationTestFunctions.h"
#include "Fit/Polyfit.h"
#include "Fit/Nnfit.h"
#include "IPC/IterativePhaseCorrelation.h"
#include "Plot/PlotCSV.h"
#include "Filtering/Filtering.h"
#include "Filtering/HistogramEqualization.h"
#include "Fractal/Fractal.h"
#include "Astrophysics/DiffrotResults.h"
#include "Astrophysics/DifferentialRotation.h"
#include "Random/Procedural.h"
#include "Random/NonMaximaSuppression.h"
#include "Random/ComplexityClassEstimation.h"
#include "Random/AoC2021D5.h"
#include "Random/AoC2021D25.h"

WindowShenanigans::WindowShenanigans(QWidget* parent) : QMainWindow(parent)
{
  // data shared within windows
  mWindowData = std::make_unique<WindowData>();

  // create windows
  mWindows["ipc"] = std::make_unique<WindowIPC>(this, mWindowData.get());
  mWindows["diffrot"] = std::make_unique<WindowDiffrot>(this, mWindowData.get());
  mWindows["features"] = std::make_unique<WindowFeatures>(this, mWindowData.get());
  mWindows["fits"] = std::make_unique<WindowFITS>(this, mWindowData.get());
  mWindows["filtering"] = std::make_unique<WindowFiltering>(this, mWindowData.get());

  // setup Qt ui - meta compiled
  ui.setupUi(this);

  // show the logo
  QPixmap pm("../data/app/logo.png");
  ui.label_2->setPixmap(pm);
  ui.label_2->setScaledContents(true);

  // set the logging text browser
  QtLogger::SetTextBrowser(ui.textBrowser);
  QtLogger::SetLogLevel(QtLogger::LogLevel::Function);

  LOG_SUCCESS("Welcome back, my friend.");

  // make signal - slot connections
  connect(ui.actionIPC, SIGNAL(triggered()), this, SLOT(ShowWindowIPC()));
  connect(ui.actionDiffrot, SIGNAL(triggered()), this, SLOT(ShowWindowDiffrot()));
  connect(ui.actionFeatures, SIGNAL(triggered()), this, SLOT(ShowWindowFeatures()));
  connect(ui.actionFITS, SIGNAL(triggered()), this, SLOT(ShowWindowFITS()));
  connect(ui.actionFiltering, SIGNAL(triggered()), this, SLOT(ShowWindowFiltering()));
  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(About()));
  connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(CloseAll()));
  connect(ui.actionSnake, SIGNAL(triggered()), this, SLOT(Snake()));
  connect(ui.actionProcedural, SIGNAL(triggered()), this, SLOT(GenerateLand()));
  connect(ui.actionUnitTests, SIGNAL(triggered()), this, SLOT(UnitTests()));
  connect(ui.pushButtonDebug, SIGNAL(clicked()), this, SLOT(RandomShit()));
}

void WindowShenanigans::ShowWindowIPC()
{
  mWindows["ipc"]->show();
}

void WindowShenanigans::ShowWindowDiffrot()
{
  mWindows["diffrot"]->show();
}

void WindowShenanigans::ShowWindowFeatures()
{
  mWindows["features"]->show();
}

void WindowShenanigans::ShowWindowFITS()
{
  mWindows["fits"]->show();
}

void WindowShenanigans::ShowWindowFiltering()
{
  mWindows["filtering"]->show();
}

void WindowShenanigans::Exit()
{
  QApplication::exit();
}

void WindowShenanigans::About()
{
  QMessageBox msgBox;
  msgBox.setText("All these shenanigans were created during my PhD studies.\n\nHave fun,\n� Zdenek Hrazdira");
  msgBox.exec();
}

void WindowShenanigans::CloseAll()
{
  cv::destroyAllWindows();
  Plot::CloseAll();

  for (auto& [windowname, window] : mWindows)
    window->close();

  LOG_INFO("All windows closed");
}

void WindowShenanigans::GenerateLand()
{
  LOG_FUNCTION("Generate land");
  cv::Mat mat = Procedural::procedural(1000, 1000);
  showimg(Procedural::colorlandscape(mat), "procedural nature");
}

void WindowShenanigans::UnitTests()
{
}

void WindowShenanigans::Snake()
{
  LOG_FUNCTION("Play snake");
}

void WindowShenanigans::closeEvent(QCloseEvent* event)
{
  /*QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Quit", "Are you sure you want to quit?    ", QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes);
  if (resBtn != QMessageBox::Yes)
  {
    event->ignore();
  }
  else
  {
    event->accept();
    CloseAll();
    LOG_SUCCESS("Good bye.");
  }*/

  CloseAll();
  LOG_INFO("Good bye.");
}

void WindowShenanigans::RandomShit()
try
{
  LOG_FUNCTION("RandomShit");
  if (0) // CC/PC dizertacka pics
  {
    cv::Mat img1 = LoadUnitFloatImage("../data/shapef.png");
    cv::Mat img2 = LoadUnitFloatImage("../data/shapesf.png");

    addnoise(img1, 0.3);
    addnoise(img2, 0.3);

    IterativePhaseCorrelation CC(img1.size(), 0, 0.5);
    IterativePhaseCorrelation PC(img1.size(), 0, 0.5);

    CC.SetDebugName("CC");
    PC.SetDebugName("PC");

    // CC.SetWindowType(IterativePhaseCorrelation::WindowType::Rectangular);
    // PC.SetWindowType(IterativePhaseCorrelation::WindowType::Rectangular);

    // CC.SetBandpassType(IterativePhaseCorrelation::BandpassType::Rectangular);
    // PC.SetBandpassType(IterativePhaseCorrelation::BandpassType::Rectangular);

    CC.SetInterpolationType(IterativePhaseCorrelation::InterpolationType::Cubic);
    PC.SetInterpolationType(IterativePhaseCorrelation::InterpolationType::Cubic);

    CC.SetL2size(15);
    PC.SetL2size(15);

    if (0)
    {
      const auto shiftCC = CC.Calculate<true, true>(img1, img2);
      LOG_DEBUG("Calculated CC shift: {}", shiftCC);
    }

    if (1)
    {
      const auto shiftPC = PC.Calculate<true, false>(img1, img2);
      LOG_DEBUG("Calculated PC shift: {}", shiftPC);
    }
  }
  if (0) // fourier inverse coeff test
  {
    cv::Mat fun = cv::Mat::zeros(4, 4, CV_32F);
    for (i32 r = 0; r < fun.rows; ++r)
      for (i32 c = 0; c < fun.cols; ++c)
        fun.at<f32>(r, c) = r + c;

    cv::Mat fft = Fourier::fft(fun);
    cv::Mat ifft = Fourier::ifft(fft);

    LOG_DEBUG(fmt::format("fun:\n{}\n", fun));
    LOG_DEBUG(fmt::format("fft:\n{}\n", fft));
    LOG_DEBUG(fmt::format("ifft:\n{}\n", ifft));
    return;
  }
  if (0) // Haar wavelet plot
  {
    cv::Mat haarX(1000, 1000, CV_32F);
    cv::Mat haarY(1000, 1000, CV_32F);

    for (i32 r = 0; r < haarX.rows; ++r)
    {
      for (i32 c = 0; c < haarX.cols; ++c)
      {
        haarX.at<f32>(r, c) = c > haarX.cols / 2 ? 1 : -1;
        haarY.at<f32>(r, c) = r > haarY.rows / 2 ? 1 : -1;
      }
    }

    Plot2D::Set("haarX");
    Plot2D::SetSavePath("Debug/haarX.png");
    Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
    Plot2D::Plot(haarX);

    Plot2D::Set("haarY");
    Plot2D::SetSavePath("Debug/haarY.png");
    Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
    Plot2D::Plot(haarY);
  }
  if (0) // ellipse fit optimization
  {
    const f64 imgsize = 1000;
    const f64 border = 500;
    cv::Point2f ellipseCenter;
    cv::Size ellipseSize;
    f32 ellipseAngle;
    cv::Mat ellipseImg;

    do
    {
      ellipseImg = cv::Mat::zeros(imgsize, imgsize, CV_32F);
      ellipseCenter = cv::Point2f(imgsize / 2 + rand11() * (imgsize / 2 + border), imgsize / 2 + rand11() * (imgsize / 2 + border));
      ellipseSize = cv::Size(std::max(rand01() * imgsize, 20.), std::max(rand01() * imgsize, 20.));
      ellipseAngle = rand11() * 90;
      ellipse(ellipseImg, cv::RotatedRect(ellipseCenter, ellipseSize, ellipseAngle), cv::Scalar(1), -1, cv::LINE_AA);
    } while (sum(ellipseImg)[0] < 1000);

    const auto f = [&](const std::vector<f64>& params) {
      cv::Point2f center(params[0], params[1]);
      cv::Size size(params[2], params[3]);
      f32 angle = params[4];
      cv::Mat img = cv::Mat::zeros(imgsize, imgsize, CV_32F);
      ellipse(img, cv::RotatedRect(center, size, angle), cv::Scalar(1), -1, cv::LINE_AA);
      auto imgDiff = cv::sum(cv::abs(img - ellipseImg))[0] / sum(ellipseImg)[0];
      auto sumDiff = cv::abs(cv::sum(img)[0] - cv::sum(ellipseImg)[0]) / cv::sum(ellipseImg)[0];
      auto cenDiff = findCentroid(img) - findCentroid(ellipseImg);
      cenDiff.x = std::abs(cenDiff.x) / img.cols;
      cenDiff.y = std::abs(cenDiff.y) / img.rows;
      const auto cost = imgDiff + cenDiff.x + cenDiff.y + sumDiff;
      return std::isfinite(cost) ? cost : std::numeric_limits<f32>::max();
    };

    Evolution evo(5);
    evo.mNP = 100;
    evo.mMutStrat = Evolution::MutationStrategy::RAND1;
    evo.mLB = {-border, -border, 1, 1, -90};
    evo.mUB = {imgsize + border, imgsize + border, imgsize, imgsize, 90};
    evo.mOptimalFitness = 1e-2;
    evo.SetParameterNames({"X", "Y", "A", "B", "R"});

    const auto res = evo.Optimize(f).optimum;
    if (res.empty())
    {
      LOG_ERROR("Optimization failed");
      return;
    }

    const cv::Point2f calcCenter(res[0], res[1]);
    const cv::Size calcSize(res[2], res[3]);
    const f32 calcAngle = res[4];

    cv::Mat ellipseImgCalculated = cv::Mat::zeros(imgsize, imgsize, CV_32F);
    ellipse(ellipseImgCalculated, cv::RotatedRect(calcCenter, calcSize, calcAngle), cv::Scalar(1), -1, cv::LINE_AA);

    auto imgDiff = cv::sum(cv::abs(ellipseImgCalculated - ellipseImg))[0] / cv::sum(ellipseImg)[0];
    auto sumDiff = cv::abs(cv::sum(ellipseImgCalculated)[0] - cv::sum(ellipseImg)[0]) / sum(ellipseImg)[0];
    auto cenDiff = findCentroid(ellipseImgCalculated) - findCentroid(ellipseImg);
    cenDiff.x = std::abs(cenDiff.x) / ellipseImg.cols;
    cenDiff.y = std::abs(cenDiff.y) / ellipseImg.rows;

    LOG_DEBUG("ImgDiff: {:.1e}, SumDiff: {:.1e}, CenDiff: {:.1e}", imgDiff, sumDiff, cenDiff.x + cenDiff.y);
    LOG_INFO("True ellipse center: [{:.1f} , {:.1f}], size: [{} , {}], angle: {:.1f}", ellipseCenter.x, ellipseCenter.y, ellipseSize.width, ellipseSize.height, ellipseAngle);
    LOG_SUCCESS("Calculated ellipse center: [{:.1f} , {:.1f}], size: [{} , {}], angle: {:.1f}", calcCenter.x, calcCenter.y, calcSize.width, calcSize.height, calcAngle);
    LOG_WARNING("Errors: center: [{:.1f} , {:.1f}], size: [{} , {}], angle: {:.1f}", abs(ellipseCenter.x - calcCenter.x), abs(ellipseCenter.y - calcCenter.y), abs(ellipseSize.width - calcSize.width),
        abs(ellipseSize.height - calcSize.height), abs(ellipseAngle - calcAngle));

    cv::Mat ellipseImgColor = cv::Mat::zeros(imgsize, imgsize, CV_32FC3);
    for (i32 r = 0; r < ellipseImgColor.rows; ++r)
    {
      for (i32 c = 0; c < ellipseImgColor.cols; ++c)
      {
        ellipseImgColor.at<cv::Vec3f>(r, c)[0] = ellipseImg.at<f32>(r, c);
        ellipseImgColor.at<cv::Vec3f>(r, c)[1] = ellipseImg.at<f32>(r, c);
        ellipseImgColor.at<cv::Vec3f>(r, c)[2] = ellipseImg.at<f32>(r, c);
      }
    }

    ellipse(ellipseImgColor, cv::RotatedRect(findCentroid(ellipseImg), cv::Size(10, 10), 0), cv::Scalar(1, 0, 0), -1, cv::LINE_AA);               // ellipse centroid
    ellipse(ellipseImgColor, cv::RotatedRect(calcCenter, calcSize, calcAngle), cv::Scalar(0.7, 0, 0.8), 3, cv::LINE_AA);                          // calc ellipse
    ellipse(ellipseImgColor, cv::RotatedRect(findCentroid(ellipseImgCalculated), cv::Size(10, 10), 0), cv::Scalar(0.7, 0, 0.8), -1, cv::LINE_AA); // calc ellipse centroid

    showimg(ellipseImgColor, "ellipse fit", 0, 0, 1, 1000);
    return;
  }
  if (0) // ellipse fit least squares
  {
    const i32 size = 100;
    cv::Point2d center(50, 50);
    std::vector<cv::Point2d> points;
    points.push_back({center.x + 10.1, center.y - 0.4});
    points.push_back({center.x + 7.2, center.y - 7.4});
    points.push_back({center.x + 0.2, center.y - 10.3});
    points.push_back({center.x + -7.1, center.y - 7.3});

    const usize pointsCount = points.size();
    cv::Mat img = cv::Mat::zeros(size, size, CV_32F);

    for (const auto& point : points)
      img.at<f32>(point.y, point.x) = 1;

    cv::Mat X(pointsCount, 5, CV_32F);
    cv::Mat Y(pointsCount, 1, CV_32F);

    for (i32 r = 0; r < X.rows; ++r)
    {
      const auto& pt = points[r];

      for (i32 c = 0; c < X.cols; ++c)
      {
        if (c == 0)
          X.at<f32>(r, c) = pt.x * pt.y;
        else if (c == 1)
          X.at<f32>(r, c) = sqr(pt.y);
        else if (c == 2)
          X.at<f32>(r, c) = pt.x;
        else if (c == 3)
          X.at<f32>(r, c) = pt.y;
        else if (c == 4)
          X.at<f32>(r, c) = 1;
      }
      Y.at<f32>(r, 0) = -sqr(pt.x);
    }

    cv::Mat Beta = (X.t() * X).inv() * X.t() * Y;

    f32 A = 1.;
    f32 B = Beta.at<f32>(0, 0);
    f32 C = Beta.at<f32>(1, 0);
    f32 D = Beta.at<f32>(2, 0);
    f32 E = Beta.at<f32>(3, 0);

    // math.stackexchange.com/a/423272
    f32 R = 0.5 * std::atan2(B, A - C);
    f32 Ar = A * sqr(cos(R)) + B * cos(R) * sin(R) + C * sqr(sin(R));
    f32 Cr = A * sqr(sin(R)) - B * cos(R) * sin(R) + C * sqr(cos(R));
    f32 Dr = D * cos(R) + E * sin(R);
    f32 Er = -D * sin(R) + E * cos(R);

    f32 centerXr = -Dr / (2. * Ar);
    f32 centerYr = -Er / (2. * Cr);

    f32 centerX = centerXr * cos(R) - centerYr * sin(R);
    f32 centerY = centerXr * sin(R) + centerYr * cos(R);

    Plot2D::Set("ellipse fit");
    Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
    Plot2D::Plot(img);

    LOG_DEBUG("Calculated ellipse center be4 rotation: [{:.1f} , {:.1f}], angle: {:.1f}", centerXr, centerYr, toDegrees(R));
    LOG_INFO("True ellipse center: [{:.1f} , {:.1f}]", center.x, center.y);
    LOG_SUCCESS("Calculated ellipse center: [{:.1f} , {:.1f}]", centerX, centerY);
  }
  if (0) // complexity estimation test
  {
    const auto f = [](const std::vector<f64>& x) {
      f64 g = 0;

      for (usize i = 0; i < 1e7; ++i) // constant portion
        g += i;

      const i32 scale = 3;
      for (usize i = 0; i < scale; ++i) // scale
        for (const auto& a : x)         // n^2
          for (const auto& b : x)
            g += a - b;

      return g;
    };
    EstimateComplexity<f64, f64>(f);
    return;
  }
  if (0) // sasko DFT test
  {
    cv::Mat img1 = roicropmid(LoadUnitFloatImage("../data/test.png"), 1000, 1000);
    cv::Mat img2;
    flip(img1, img2, 1);
    rotate(img2, img2, cv::ROTATE_90_COUNTERCLOCKWISE);
    Plot2D::Plot("img1", img1);
    Plot2D::Plot("img2", img2);
    cv::Mat fft1 = Fourier::fft(img1);
    cv::Mat fft2 = Fourier::fft(img2);

    transpose(fft2, fft2);

    Fourier::fftshift(fft1);
    Fourier::fftshift(fft2);
    Plot2D::Plot("fft1", Fourier::logmagn(fft1));
    Plot2D::Plot("fft2", Fourier::logmagn(fft2));
    Plot2D::Plot("fft diff", abs(Fourier::logmagn(fft2) - Fourier::logmagn(fft1)));

    cv::Mat planes1[2], planes2[2];
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
    cv::Mat img1 = LoadUnitFloatImage("../articles/swind/source/1/cropped/crop1.png");
    cv::Mat img2 = LoadUnitFloatImage("../articles/swind/source/1/cropped/crop2.png");

    i32 sajz = 0;
    cv::Size size(sajz, sajz);
    if (sajz > 0 and img1.size() != size)
    {
      cv::resize(img1, img1, size);
      cv::resize(img2, img2, size);
    }

    bool crop = true;
    if (crop)
    {
      i32 cropsize = 128;
      img1 = roicrop(img1, img1.cols * 0.6, img1.rows * 0.63, cropsize, cropsize);
      img2 = roicrop(img2, img2.cols * 0.6, img2.rows * 0.63, cropsize, cropsize);
    }

    IterativePhaseCorrelation ipc = *mWindowData->IPC;
    ipc.SetSize(img1.rows, img1.cols);
    // ipc.SetWindowType(IterativePhaseCorrelation::WindowType::Hann);
    // ipc.SetBandpassType(IterativePhaseCorrelation::BandpassType::Gaussian);

    const auto shiftCalc = ipc.Calculate(img1, img2);
    LOG_DEBUG("IPC shift: [{}]", shiftCalc);
    return;
  }
  if (0) // swind crop
  {
    std::string path = "../articles/swind/source/2";
    i32 sizeX = 1500;
    i32 sizeY = 1000;

    for (usize i = 0; i < 11; i++)
    {
      auto pic = cv::imread(fmt::format("{}/raw/0{}_calib.PNG", path, i + 1), cv::IMREAD_ANYDEPTH);
      pic = roicrop(pic, 0.33 * pic.cols, 0.69 * pic.rows, sizeX, sizeY);
      saveimg(fmt::format("{}/cropped/crop{}.PNG", path, i), pic);
    }
  }
  if (0) // ipc align test
  {
    // silocary @1675/2200

    cv::Point cropfocus(2048, 2048);
    i32 cropsize = 1.0 * 4096;

    cv::Mat img1 = roicrop(LoadUnitFloatImage("../data/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);
    cv::Mat img2 = roicrop(LoadUnitFloatImage("../data/171A.png"), cropfocus.x, cropfocus.y, cropsize, cropsize);

    i32 size = cropsize;
    cv::resize(img1, img1, cv::Size(size, size));
    cv::resize(img2, img2, cv::Size(size, size));

    Shift(img2, -950, 1050);
    Rotate(img2, 70, 1.2);

    IterativePhaseCorrelation ipc = *mWindowData->IPC;
    ipc.SetSize(img1.size());
    cv::Mat aligned = ipc.Align(img1, img2);
    showimg(std::vector<cv::Mat>{img1, img2, aligned}, "align triplet");
  }
  if (0) // dft vs gpu::dft
  {
    const i32 minsize = 16;
    const i32 maxsize = 512;
    const i32 sizeStep = 10;
    const i32 iters = (maxsize - minsize + 1) / sizeStep % 2 ? (maxsize - minsize + 1) / sizeStep : (maxsize - minsize + 1) / sizeStep + 1;
    const i32 itersPerSize = 100;
    cv::Mat img = LoadUnitFloatImage("../data/AIA/171A.png");
    std::vector<f64> sizes(iters);
    std::vector<f64> timeCpu(iters);
    std::vector<f64> timeGpu(iters);
    cv::Mat fft = Fourier::gpufft(img); // init

    for (usize i = 0; i < iters; ++i)
    {
      i32 size = minsize + (f32)i / (iters - 1) * (maxsize - minsize);
      sizes[i] = size;
      LOG_INFO("{} / {} Calculating FFT speeds for image size {}", i + 1, iters, size);
      cv::Mat resized = roicrop(img, img.cols / 2, img.rows / 2, size, size);

      {
        auto start = std::chrono::high_resolution_clock::now();

        for (i32 it = 0; it < itersPerSize; ++it)
          cv::Mat fft_ = Fourier::fft(resized);

        auto end = std::chrono::high_resolution_clock::now();
        timeCpu[i] = (f64)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / itersPerSize;
      }

      {
        auto start = std::chrono::high_resolution_clock::now();

        for (i32 it = 0; it < itersPerSize; ++it)
          cv::Mat fft_ = Fourier::gpufft(resized);

        auto end = std::chrono::high_resolution_clock::now();
        timeGpu[i] = (f64)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / itersPerSize;
      }
    }

    Plot1D::Set("FFT CPU vs GPU");
    Plot1D::SetYnames({"fft cpu", "fft gpu"});
    Plot1D::SetYlabel("time per DFT [ms]");
    Plot1D::SetXlabel("image size");
    Plot1D::Plot("FFT CPU vs GPU", sizes, {timeCpu, timeGpu});
    return;
  }
  if (0) // cuda
  {
    cv::Mat img = LoadUnitFloatImage("../data/test.png");
    cv::Mat fft, fftpacked, gpufft;
    cv::Mat TB, TBpacked, cuTB;
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
      TBpacked = Fourier::ifft(fftpacked);
    }
    if (gpu)
    {
      LOG_FUNCTION("gpufft");
      gpufft = Fourier::gpufft(img);
      cuTB = Fourier::gpuifft(gpufft);
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
      Plot2D::Plot("gpufft logmagn", Fourier::logmagn(gpufft));
      Plot2D::Plot("cuTB", cuTB);
    }

    // Plot2D::Plot("gpufft-fft logmagn absdiff", abs(Fourier::logmagn(gpufft) - Fourier::logmagn(fft)));
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
    cv::Mat mat(100, 100, CV_32F);
    roicrop(mat, 4, 4, 11, 11);
    return;
  }
  if (0) // non maxima suppression
  {
    cv::Mat img = LoadUnitFloatImage("../data/test.png");
    f32 scale = 1.0;
    cv::resize(img, img, cv::Size(scale * img.cols, scale * img.rows));
    NonMaximaSuppresion(img);
  }
  if (0) // kirkl test
  {
    for (i32 i = 5; i < 21; i += 2)
    {
      cv::Mat kirklik;
      cv::resize(kirkl(i), kirklik, cv::Size(999, 999), 0, 0, cv::INTER_NEAREST);
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
      for (usize i = 0; i < paramsPieces.size(); ++i)
        LOG_ERROR("paramsPieces[{}]: {}", i, paramsPieces[i].str());

    std::regex flagsRegex(R"(.+IS_([A-Za-z]+)\[=([A-Za-z]+)\];)");
    LOG_ERROR("Input string: {}, flags regex: {}", str, std::regex_match(str, flagsRegex));
    std::smatch flagsPieces;
    if (std::regex_match(str, flagsPieces, flagsRegex))
      for (usize i = 0; i < flagsPieces.size(); ++i)
        LOG_ERROR("flagsPieces[{}]: {}", i, flagsPieces[i].str());

    std::regex e(R"(IS_([A-Za-z]+)\[=([A-Za-z]+)\];)");
    i32 submatches[] = {1, 2};
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
    i32 rows = 1000;
    i32 cols = 1000;

    cv::Mat bandpassR = cv::Mat::zeros(rows, cols, CV_32F);
    cv::Mat bandpassG = cv::Mat::zeros(rows, cols, CV_32F);
    cv::Mat gaussL = cv::Mat::zeros(rows, cols, CV_32F);
    cv::Mat gaussH = cv::Mat::zeros(rows, cols, CV_32F);

    f32 fL = 0.2;
    f32 fH = 0.8;

    f32 sL = 1.0;
    f32 sH = 0.01;

    for (i32 r = 0; r < rows; ++r)
    {
      for (i32 c = 0; c < cols; ++c)
      {
        f32 R = sqrt(std::pow(((f32)c - cols / 2) / (cols / 2), 2) + std::pow(((f32)r - rows / 2) / (rows / 2), 2));
        bandpassR.at<f32>(r, c) = (fL <= R and R <= fH) ? 1 : 0;

        gaussL.at<f32>(r, c) = exp(-std::pow(((f32)c - cols / 2) / (cols / 2), 2) * std::pow(sL, 2) - std::pow(((f32)r - rows / 2) / (rows / 2), 2) * std::pow(sL, 2));

        gaussH.at<f32>(r, c) = 1.0 - exp(-std::pow(((f32)c - cols / 2) / (cols / 2), 2) / std::pow(sH, 2) - std::pow(((f32)r - rows / 2) / (rows / 2), 2) / std::pow(sH, 2));

        bandpassG.at<f32>(r, c) = gaussL.at<f32>(r, c) * gaussH.at<f32>(r, c);
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
    cv::Mat img = cv::imread("C:\\Users\\Zdeny\\Downloads\\src.jpg", cv::IMREAD_GRAYSCALE);
    img.convertTo(img, CV_32F);

    cv::Mat ref = cv::imread("C:\\Users\\Zdeny\\Downloads\\ref.tif", cv::IMREAD_GRAYSCALE);
    ref.convertTo(ref, CV_32F);

    cv::Mat out = img.clone();
    i32 shiftX = 800;
    i32 shiftY = 83;

    auto MyModulo = [](i32 a, i32 b) { return a - a / b * b >= 0 ? a - a / b * b : a - a / b * b + b; };

    for (i32 r = 0; r < img.rows; ++r)
    {
      for (i32 c = 0; c < img.cols; ++c)
      {
        i32 srcrow = MyModulo(r - shiftY, img.rows);
        i32 srccol = MyModulo(c - shiftX, img.cols);
        out.at<f32>(r, c) = img.at<f32>(srcrow, srccol);
      }
    }

    showimg(std::vector<cv::Mat>{img, out, ref}, "cyclic shift");
    // Plot2D::Plot(abs(ref - out), "diff plot");
    // Plot2D::Plot(img, "img plot");
  }
  if (0) // histogram equalize
  {
    cv::Mat img = cv::imread("../data/test.png", cv::IMREAD_GRAYSCALE);
    normalize(img, img, 0, 255, cv::NORM_MINMAX);
    img.convertTo(img, CV_8UC1);
    cv::resize(img, img, cv::Size(500, 500));
    cv::Mat heq = EqualizeHistogram(img);
    cv::Mat aheq = EqualizeHistogramAdaptive(img, 201);

    ShowHistogram(img, "img histogram");
    ShowHistogram(heq, "heq histogram");
    ShowHistogram(aheq, "aheq histogram");

    showimg(std::vector<cv::Mat>{img, heq, aheq}, "hist eq", false, 0, 1, 1000);
  }
  if (0) // ipc optimize test
  {
    IterativePhaseCorrelation ipc(256, 256);
    ipc.Optimize("../data/", "../data/", 0.3, 0.01, 11);
  }
  if (0) // plot from csv file
  {
    PlotCSV::plot("E:\\WindowShenanigans\\articles\\tokens\\data\\tokens1.csv", "E:\\WindowShenanigans\\articles\\tokens\\plots\\tokens1.png");
    PlotCSV::plot("E:\\WindowShenanigans\\articles\\tokens\\data\\tokens2.csv", "E:\\WindowShenanigans\\articles\\tokens\\plots\\tokens2.png");
  }
  if (0) // plot pen colors
  {
    i32 ncurves = 30;
    i32 ndata = 100;

    auto x = zerovect(ndata);
    auto ys = zerovect2(ncurves, ndata);

    for (i32 d = 0; d < ndata; d++)
      x[d] = d;

    for (i32 c = 0; c < ncurves; c++)
      for (i32 d = 0; d < ndata; d++)
        ys[c][d] = sin((f64)d / ndata * Constants::TwoPi + (f64)c / ncurves * Constants::Pi);

    // Plot1D::Plot(x, ys, "pen colors");
  }
  if (0) // Plot1D + Plot2D test
  {
    // 1D
    i32 N = 1000;
    auto X = zerovect(N);
    auto Y1s = zerovect2(3, N, 0.);
    auto Y2s = zerovect2(2, N, 0.);
    f64 s1 = rand01();
    f64 s2 = rand01();
    f64 s3 = rand01();

    for (i32 x = 0; x < N; x++)
    {
      X[x] = x;

      Y1s[0][x] = s1 * 100.0 * sin(Constants::TwoPi * (f64)x / N);
      Y1s[1][x] = s2 * 200.0 * sin(Constants::TwoPi * (f64)x / N);
      Y1s[2][x] = s3 * 500.0 * sin(Constants::TwoPi * (f64)x / N);

      Y2s[0][x] = s1 * 1.0 * cos(Constants::TwoPi * (f64)x / N);
      Y2s[1][x] = s2 * 0.5 * cos(Constants::TwoPi * (f64)x / N);
    }

    // 2D
    usize Ny = 1000;
    usize Nx = 1000;
    auto Z = zerovect2(Ny, Nx, 0.);

    for (usize y = 0; y < Ny; y++)
      for (usize x = 0; x < Nx; x++)
        Z[y][x] = sin((f64)x * y / Nx / Ny * 100) * rand();

    // Plot1D::Plot(X, Y1s, Y2s, "very nice plot", "X", "Y1", "Y2", std::vector<std::string>{"y1a", "y1b", "y1c"}, std::vector<std::string>{"y2a", "y2b"});
    // Plot2D::Plot(Z, "niceplot", "X", "Y", "Z", 0, 1, 0, 1, 2);
  }
  if (0) // 2d poylfit
  {
    usize size = 100;
    usize size2 = 1000;
    usize trials = 500;
    i32 degree = 5;
    std::vector<f64> xdata(trials);
    std::vector<f64> ydata(trials);
    std::vector<f64> zdata(trials);
    std::vector<cv::Point2f> pts(trials);
    cv::Mat pointiky = cv::Mat::zeros(size2, size2, CV_8UC3);

    cv::Mat orig = cv::Mat::zeros(size, size, CV_32F);
    for (usize r = 0; r < size; r++)
    {
      for (usize c = 0; c < size; c++)
      {
        f64 x = (f32)c / 99;
        f64 y = (f32)r / 99;
        orig.at<f32>(r, c) = sqr(x - 0.75) + sqr(y - 0.25) + 0.1 * sin(0.1 * (x * 99 + y * 99) + 1);
      }
    }

    for (usize i = 0; i < trials; i++)
    {
      xdata[i] = rand01();
      ydata[i] = rand01();
      pts[i] = cv::Point2f(xdata[i], ydata[i]);
      zdata[i] = sqr(xdata[i] - 0.75) + sqr(ydata[i] - 0.25) + 0.1 * sin(0.1 * (xdata[i] * 99 + ydata[i] * 99) + 1);
      drawPoint(pointiky, static_cast<i32>(size2) * pts[i], cv::Scalar(rand() % 256, rand() % 256, rand() % 256), 5, 3);
    }

    cv::Mat fitPoly = polyfit(xdata, ydata, zdata, degree, 0, 1, 0, 1, size, size);
    cv::Mat fitNn = nnfit(pts, zdata, 0, 1, 0, 1, size, size);
    cv::Mat fitwNn = wnnfit(pts, zdata, 0, 1, 0, 1, size, size);

    showimg(fitPoly, "fit - polynomial5", true);
    showimg(fitNn, "fit - nearest neighbor", true);
    showimg(fitwNn, "fit - weighted nearest neighbors", true);
    showimg(orig, "original", true);
    showimg(pointiky, "trials");
  }
  if (0) // ipc sign test
  {
    cv::Mat img1 = LoadUnitFloatImage("../../data/Shapes/shape.png");
    cv::Mat img2 = LoadUnitFloatImage("../../data/Shapes/shapes.png");

    IterativePhaseCorrelation ipc(img1.rows, img1.cols, 0.1, 200);

    auto shift = ipc.Calculate<true, false>(img1, img2);

    LOG_INFO("shift = {}", shift);
  }
  if (0) // loadfits test
  {
    LoadUnitFloatImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_00_22__CONT.fits");
    LoadUnitFloatImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_01_07__CONT.fits");
    LoadUnitFloatImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_19_07__CONT.fits");
    LoadUnitFloatImage("D:\\SDOpics\\Calm2020stride25\\2020_01_01__00_19_52__CONT.fits");
    LoadUnitFloatImage("D:\\SDOpics\\Calm2020stride25\\2020_02_02__13_16_08__CONT.fits");
  }
  if (0) // loadfits test 2
  {
    auto pic = LoadUnitFloatImage("D:\\SDOpics\\Calm2020stride25\\2020_01_02__18_49_52__CONT.fits");
    showimg(pic, "pic");
  }
  if (0) // 1D / 2D sorted xs interp test
  {
    std::vector<f64> xs{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<f64> ys{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
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

    std::vector<f64> xs1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<f64> ys1{0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    std::vector<f64> xs2{-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<f64> ys2{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    std::vector<std::vector<f64>> xss = {xs1, xs2};
    std::vector<std::vector<f64>> yss = {ys1, ys2};
    LOG_INFO("Diffrot interp 2D a) {}", DiffrotResults::Interpolate(xss, yss, 1.0));
    LOG_INFO("Diffrot interp 2D b) {}", DiffrotResults::Interpolate(xss, yss, -1.0));
    LOG_INFO("Diffrot interp 2D c) {}", DiffrotResults::Interpolate(xss, yss, 1.5));
    LOG_INFO("Diffrot interp 2D d) {}", DiffrotResults::Interpolate(xss, yss, 1.75));
  }
  if (0) // try/catch performance test
  {
    i32 N = 1e7;
    i32 iters = 10;
    std::vector<f64> vec(N);

    for (i32 iter = 0; iter < iters; ++iter)
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
  if (0)
  {
    Fractalset fractalSet;
    computeFractal(fractalSet);
  }
  if (0) // AoC2021
  {
    // LOG_SUCCESS("AoC2021D5: {}", AoC2021D5());
    LOG_SUCCESS("AoC2021D5: {}", AoC2021D25::AoC2021D25());
  }
  if (0) // faims TCGFR plot
  {
    static constexpr usize N = 501;
    static constexpr f64 tempdiffMin = -2;
    static constexpr f64 tempdiffMax = 7;
    static constexpr f64 maxtempdiff = 5;
    static constexpr f64 mintgcfr = 3;
    static constexpr f64 maxtgcfr = 13.5;

    std::vector<f64> tempdiff(N);
    std::vector<f64> tcgfr1(N);
    std::vector<f64> tcgfr2(N);
    std::vector<f64> tcgfr3(N);

    for (usize i = 0; i < N; ++i)
    {
      tempdiff[i] = tempdiffMin + static_cast<f32>(i) / (N - 1) * (tempdiffMax - tempdiffMin);
      tcgfr1[i] = mintgcfr + std::pow(std::clamp(tempdiff[i], 0.0, maxtempdiff) / maxtempdiff, 1) * (maxtgcfr - mintgcfr);
      tcgfr2[i] = mintgcfr + std::pow(std::clamp(tempdiff[i], 0.0, maxtempdiff) / maxtempdiff, 2) * (maxtgcfr - mintgcfr);
      tcgfr3[i] = mintgcfr + std::pow(std::clamp(tempdiff[i], 0.0, maxtempdiff) / maxtempdiff, 3) * (maxtgcfr - mintgcfr);
    }

    Plot1D::Set("TCGFR");
    Plot1D::SetXlabel("Current maximum temperature difference [°C]");
    Plot1D::SetYlabel("Target cooling gas flow rate [lpm]");
    Plot1D::SetYmin(0);
    Plot1D::SetYmax(15);
    Plot1D::SetYnames({"linear", "pow2", "pow3"});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::SetSavePath("D:/tfs/faims/tcgfr.png");
    Plot1D::Plot(tempdiff, {tcgfr1, tcgfr2, tcgfr3});
  }
  if (0) // png imread
  {
    cv::Mat img1 = cv::imread("../../data/FITS/HMI.png", cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
    Plot2D::Set("HMI1.png");
    Plot2D::Plot(img1);
    Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
  }
  if (0) // lexa roll question
  {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distrib(1, 100);
    u64 samples = 1e9;
    u64 win1 = 0;
    u64 win2 = 0;
    u64 win3 = 0;

    for (u64 n = 0; n < samples; ++n)
    {
      const auto roll1 = distrib(rng) + 50; // strong enemy
      const auto roll2 = distrib(rng);      // normal enemy
      const auto roll3 = distrib(rng);      // me

      if (roll1 > roll2 and roll1 > roll3)
        win1++;
      if (roll2 > roll1 and roll2 > roll3)
        win2++;
      if (roll3 > roll1 and roll3 > roll2)
        win3++;
    }

    LOG_DEBUG("Roll win1: {}%, win2: {}%, win3: {}%, sum: {}%", static_cast<f64>(win1) * 100 / samples, static_cast<f64>(win2) * 100 / samples, static_cast<f64>(win3) * 100 / samples,
        static_cast<f64>(win1 + win2 + win3) * 100 / samples);
  }
  if (0) // matplotlib
  {
    /*
    usize n = 1000;
    std::vector<f64> x, y1, y2;
    x.reserve(n);
    y1.reserve(n);
    y2.reserve(n);

    plt::figure_size(1200, 780);
    plt::title("title xd");
    plt::xlabel("x hehe");
    plt::ylabel("y hehe");
    plt::legend();
    // plt::xlim(0., 0.7 * n);

    for (usize i = 0; i < n; ++i)
    {
      x.push_back(i);
      y1.push_back(sin(6. * 2. * Constants::Pi / n * i));
      y2.push_back(cos(6. * 2. * Constants::Pi / n * i));

      plt::plot(x, y1, {{"color", "blue"}, {"linestyle", "--"}, {"label", "y1"}});
      plt::plot(x, y2, {{"color", "green"}, {"linestyle", "--"}, {"label", "y2"}});
      plt::show(false); // show the figure instead of saving it
    }

    // plt::savefig("standard.pdf"); // save the figure
    */
  }
  if (0) // ipc perf test
  {
    auto ipc = *mWindowData->IPC;
    const auto img = cv::imread("../test/data/shape.png", cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
    const auto crop1 = roicrop(img, img.cols / 2, img.rows / 2, ipc.GetCols(), ipc.GetRows());
    const auto crop2 = roicrop(img, img.cols / 2 + 1, img.rows / 2 + 1, ipc.GetCols(), ipc.GetRows());
    usize n = 1000;
    for (usize i = 0; i < n; ++i)
      std::ignore = ipc.Calculate(crop1, crop2);
  }
  if (0) // 1D plot animation
  {
    u32 n = 1000;

    Plot1D::Set("Test1D");
    Plot1D::SetYnames({"y1", "y2"});

    for (u32 i = 0; i < n; ++i)
    {
      f64 x = (f64)i / (n - 1);
      f64 y1 = std::sin(8 * 6.28 * x) * std::abs(std::cos(6.28 * x));
      f64 y2 = std::cos(8 * 6.28 * x) * std::abs(std::cos(6.28 * x));
      Plot1D::Plot(x, {y1, y2});
    }
  }
  if (0) // 2D plot animation
  {
    i32 n = 1000;
    i32 nt = 500;
    cv::Mat mat = cv::Mat::zeros(n, n, CV_32F);
    Plot2D::Set("Test2D");

    for (i32 t = 0; t < nt; ++t)
    {
      mat.at<f32>(0, 0) = -1;
      mat.at<f32>(0, 1) = 1;
      for (i32 r = 0; r < n; ++r)
        for (i32 c = 2; c < n; ++c)
          mat.at<f32>(r, c) = std::sin(20. * (f32)r / n) * std::sin(20. * (f32)c / n) * std::sin(20. * (f32)t / nt);

      Plot2D::Plot(mat);
    }
  }
  if (0) // polyfit / trigfit test
  {
    i32 n = 101;
    f64 c0 = 3.23, c1 = 2.56, c2 = 1.65;
    std::vector<f64> x(n), y(n);

    for (i32 i = 0; i < n; ++i)
    {
      x[i] = static_cast<f64>(i) / (n - 1) * 6.28;
      y[i] = c0 + c1 * std::pow(std::sin(x[i]), 2) + c2 * std::pow(std::sin(x[i]), 4);
    }

    const auto fy = sin2sin4fit(x, y);
    Plot1D::Set("Polyfit test");
    Plot1D::SetYnames({"y", "y fit"});
    Plot1D::SetY2names({"f fit - y"});
    Plot1D::Plot(x, {y, fy}, {fy - y});
  }
  if (0) // pybind+matplotlib test
  {
    i32 n = 101;
    std::vector<double> x(n);
    std::vector<double> y1(n);
    std::vector<double> y2(n);

    for (i32 i = 0; i < n; ++i)
    {
      x[i] = static_cast<f64>(i) / (n - 1) * 6.28;
      y1[i] = std::sin(x[i]);
      y2[i] = std::cos(x[i]) * 1000;
    }

    py::dict locals;
    locals["x"] = x;
    locals["y1"] = y1;
    locals["y2"] = y2;

    py::eval_file("../script/plot/plot.py", locals);
  }
  if (1) // pybind+matplotlib wrap test
  {
    i32 n = 101;
    std::vector<double> x(n);
    std::vector<double> y(n);
    std::vector<double> y2(n);
    std::vector<double> y3(n);
    std::vector<double> y4(n);

    for (i32 i = 0; i < n; ++i)
    {
      x[i] = static_cast<f64>(i) / (n - 1) * 6.28;
      y[i] = std::sin(x[i]);
      y2[i] = std::cos(x[i]) * 5;
      y3[i] = std::exp(x[i]);
      y4[i] = std::exp(x[i]) + 120;
    }

    Plt::Plot("plot", {.x = x, .y = y, .title = "x/y"});
    Plt::Plot("plot", {.x = x, .y = y, .y2 = y2, .label_y = "y", .label_y2 = "y2", .title = "x/y+y2"});
    Plt::Plot("plot", {.x = x, .ys = {y, y2}, .label_ys = {"y", "y2"}, .title = "x/ys"});
    Plt::Plot("plot", {.x = x, .ys = {y, y2}, .y2s = {y3, y4}, .label_ys = {"y", "y2"}, .label_y2s = {"y3", "y4"}, .linestyle_ys = {"b-", "g-"}, .linestyle_y2s = {"r--", "k-."}, .title = "x/ys+y2s"});

    return;
  }
  if (0) // optimization / metaoptimization
  {
    const i32 N = 2;
    const i32 runs = 20;
    const i32 maxFunEvals = 1000;
    const f32 optimalFitness = -Constants::Inf;
    Evolution Evo(N);
    Evo.mNP = 5 * N;
    Evo.mMutStrat = Evolution::RAND1;
    Evo.mLB = zerovect(N, -5.0);
    Evo.mUB = zerovect(N, +5.0);
    Evo.mMaxFunEvals = maxFunEvals;
    Evo.mOptimalFitness = optimalFitness;
    Evo.SetName("debug");
    Evo.SetParameterNames({"x", "y"});
    Evo.SetConsoleOutput(true);
    Evo.SetPlotOutput(true);
    Evo.SetPlotObjectiveFunctionLandscape(true);
    Evo.SetPlotObjectiveFunctionLandscapeIterations(51);
    Evo.SetSaveProgress(true);

    if (0)
      Evo.MetaOptimize(OptimizationTestFunctions::Rosenbrock, Evolution::ObjectiveFunctionValue, runs, maxFunEvals, optimalFitness);
    else
      Evo.Optimize(OptimizationTestFunctions::Rosenbrock);
  }
  if (1) // ipc debug stuff
  {
    auto& window = dynamic_cast<WindowIPC&>(*mWindows["ipc"]);
    window.show();
    window.ShowDebugStuff();
  }
}
catch (const std::exception& e)
{
  LOG_ERROR("RandomShit() error: {}", e.what());
}
catch (...)
{
  LOG_ERROR("RandomShit() error: {}", "Unknown error");
}