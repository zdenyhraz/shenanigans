
#include "ipcAux.h"

void alignPics(const cv::Mat& input1, const cv::Mat& input2, cv::Mat& output, IPCsettings set)
{
  cv::Mat estR, estT;
  cv::Point2f center((f32)input1.cols / 2, (f32)input1.rows / 2);
  output = input2.clone();

  if (1) // calculate rotation and scale
  {
    cv::Mat img1FT = fourier(input1);
    cv::Mat img2FT = fourier(input2);
    img1FT = quadrantswap(img1FT);
    img2FT = quadrantswap(img2FT);
    cv::Mat planes1[2] = {cv::Mat::zeros(img1FT.size(), CV_32F), cv::Mat::zeros(img1FT.size(), CV_32F)};
    cv::Mat planes2[2] = {cv::Mat::zeros(img1FT.size(), CV_32F), cv::Mat::zeros(img1FT.size(), CV_32F)};
    split(img1FT, planes1);
    split(img2FT, planes2);
    cv::Mat img1FTm, img2FTm;
    magnitude(planes1[0], planes1[1], img1FTm);
    magnitude(planes2[0], planes2[1], img2FTm);
    bool logar = true;
    if (logar)
    {
      img1FTm += cv::Scalar::all(1.);
      img2FTm += cv::Scalar::all(1.);
      log(img1FTm, img1FTm);
      log(img2FTm, img2FTm);
    }
    normalize(img1FTm, img1FTm, 0, 1, cv::NORM_MINMAX);
    normalize(img2FTm, img2FTm, 0, 1, cv::NORM_MINMAX);
    cv::Mat img1LP, img2LP;
    f64 maxRadius = 1. * std::min(center.y, center.x);
    warpPolar(img1FTm, img1LP, cv::Size(input1.cols, input1.rows), center, maxRadius, cv::INTER_LINEAR + cv::WARP_FILL_OUTLIERS + cv::WARP_POLAR_LOG); // semilog Polar
    warpPolar(img2FTm, img2LP, cv::Size(input1.cols, input1.rows), center, maxRadius, cv::INTER_LINEAR + cv::WARP_FILL_OUTLIERS + cv::WARP_POLAR_LOG); // semilog Polar
    auto LPshifts = phasecorrel(img1LP, img2LP, set);
    std::cout << "LPshifts: " << LPshifts << std::endl;
    f64 anglePredicted = -LPshifts.y / input1.rows * 360;
    f64 scalePredicted = exp(LPshifts.x * log(maxRadius) / input1.cols);
    std::cout << "Evaluated rotation: " << anglePredicted << " deg" << std::endl;
    std::cout << "Evaluated scale: " << 1. / scalePredicted << " " << std::endl;
    estR = getRotationMatrix2D(center, -anglePredicted, scalePredicted);
    warpAffine(output, output, estR, cv::Size(input1.cols, input1.rows));
  }

  if (1) // calculate shift
  {
    auto shifts = phasecorrel(input1, output, set);
    std::cout << "shifts: " << shifts << std::endl;
    f64 shiftXPredicted = shifts.x;
    f64 shiftYPredicted = shifts.y;
    std::cout << "Evaluated shiftX: " << shiftXPredicted << " px" << std::endl;
    std::cout << "Evaluated shiftY: " << shiftYPredicted << " px" << std::endl;
    estT = (cv::Mat_<f32>(2, 3) << 1., 0., -shiftXPredicted, 0., 1., -shiftYPredicted);
    warpAffine(output, output, estT, cv::Size(input1.cols, input1.rows));
  }
}

cv::Mat AlignStereovision(const cv::Mat& img1In, const cv::Mat& img2In)
{
  cv::Mat img1 = img1In.clone();
  cv::Mat img2 = img2In.clone();
  normalize(img1, img1, 0, 65535, cv::NORM_MINMAX);
  normalize(img2, img2, 0, 65535, cv::NORM_MINMAX);
  img1.convertTo(img1, CV_16U);
  img2.convertTo(img2, CV_16U);

  f64 d = 0.0;
  f64 g = 0.5;

  std::vector<f64> rgb1 = {1. - d, g, d};
  std::vector<f64> rgb2 = {d, 1. - g, 1. - d};

  std::vector<cv::Mat> channels1(3);
  std::vector<cv::Mat> channels2(3);
  cv::Mat img1CLR, img2CLR;

  channels1[0] = rgb1[2] * img1;
  channels1[1] = rgb1[1] * img1;
  channels1[2] = rgb1[0] * img1;

  channels2[0] = rgb2[2] * img2;
  channels2[1] = rgb2[1] * img2;
  channels2[2] = rgb2[0] * img2;

  merge(channels1, img1CLR);
  merge(channels2, img2CLR);

  cv::Mat result = img1CLR + img2CLR;
  normalize(result, result, 0, 65535, cv::NORM_MINMAX);

  return result;
}

void alignPicsDebug(const cv::Mat& img1In, const cv::Mat& img2In, IPCsettings& IPC_settings)
{
  /*
  FitsParams params;
  cv::Mat img1 = loadfits("D:\\MainOutput\\304A.fits", params, fitsType::AIA);
  cv::Mat img2 = loadfits("D:\\MainOutput\\171A.fits", params, fitsType::AIA);
  */

  cv::Mat img1 = img1In.clone();
  cv::Mat img2 = img2In.clone();

  filterSettings filterSet1(10, 4500, 1);
  filterSettings filterSet2(10, 0, 1);
  filterSettings filterSetASV(1, 0, 1);

  img1 = filterContrastBrightness(img1, filterSet1.contrast, filterSet1.brightness);
  img2 = filterContrastBrightness(img2, filterSet2.contrast, filterSet2.brightness);

  img1 = gammaCorrect(img1, filterSet1.gamma);
  img2 = gammaCorrect(img2, filterSet2.gamma);

  cv::Point2f center((f32)img1.cols / 2, (f32)img1.rows / 2);
  bool artificial = true;
  if (artificial)
  {
    // artificial transform for testing
    // img2 = img1.clone();
    f64 angle = 70;    // 70;
    f64 scale = 1.2;   // 1.2;
    f64 shiftX = -950; //-958;
    f64 shiftY = 1050; // 1050;

    std::cout << "Artificial parameters:" << std::endl << "Angle: " << angle << std::endl << "Scale: " << scale << std::endl << "ShiftX: " << shiftX << std::endl << "ShiftY: " << shiftY << std::endl;
    cv::Mat R = getRotationMatrix2D(center, angle, scale);
    cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
    warpAffine(img2, img2, T, cv::Size(img1.cols, img1.rows));
    warpAffine(img2, img2, R, cv::Size(img1.cols, img1.rows));
  }

  showimg(img1, "src1");
  showimg(img2, "src2");
  if (0)
    saveimg("D:\\MainOutput\\align\\src1.png", img1);
  if (0)
    saveimg("D:\\MainOutput\\align\\src2.png", img2);

  // show misaligned pics
  showimg(gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma), "not aligned_stereo");
  if (0)
    saveimg("D:\\MainOutput\\align\\ASV_notAligned.png", gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma));

  // now align them
  alignPics(img1, img2, img2, IPC_settings);

  // show aligned pics
  showimg(gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma), "aligned_stereo");
  if (0)
    saveimg("D:\\MainOutput\\align\\ASV_Aligned.png", gammaCorrect(filterContrastBrightness(AlignStereovision(img1, img2), filterSetASV.contrast, filterSetASV.brightness), filterSetASV.gamma));

  std::cout << "Finito senor" << std::endl;
}

void registrationDuelDebug(IPCsettings& IPC_settings1, IPCsettings& IPC_settings2)
{
  if (IPC_settings1.getcols() == 0 or IPC_settings2.getcols() == 0)
  {
    std::cout << "> please initialize IPC_settings1 and IPC_settings2 to run benchmarks" << std::endl;
    return;
  }
  // initialize main parameters
  f64 artificialShiftRange = IPC_settings1.getcols() / 8;
  i32 trialsPerStartPos = 1000;
  std::vector<f64> startFractionX = {0.5};
  std::vector<f64> startFractionY = {0.5};
  i32 progress = 0;

  // load the test picture
  FitsParams params;
  cv::Mat src1 = loadfits("D:\\MainOutput\\HMI.fits", params);
  // cv::Mat src1 = cv::imread("D:\\MainOutput\\png\\HMI_proc.png", IMREAD_ANYDEPTH);
  cv::Mat edgemaska = edgemask(IPC_settings1.getcols(), IPC_settings1.getcols());

  // initialize .csv output
  std::ofstream listing("D:\\MainOutput\\CSVs\\tempBenchmarks.csv", std::ios::out | std::ios::trunc);
  listing << IPC_settings1.getcols() << std::endl;

#pragma omp parallel for
  for (usize startPos = 0; startPos < startFractionX.size(); startPos++)
  {
    // testing shifts @different picture positions
    i32 startX = src1.cols * startFractionX[startPos];
    i32 startY = src1.rows * startFractionY[startPos];
#pragma omp parallel for
    for (i32 trial = 0; trial < trialsPerStartPos; trial++)
    {
      // shift entire src2 pic
      cv::Mat src2;
      f64 shiftX = (f64)trial / trialsPerStartPos * artificialShiftRange;
      f64 shiftY = 0;
      cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., shiftX, 0., 1., shiftY);
      warpAffine(src1, src2, T, cv::Size(src1.cols, src1.rows));

      // crop both pics
      cv::Mat crop1 = roicrop(src1, startX, startY, IPC_settings1.getcols(), IPC_settings1.getcols());
      cv::Mat crop2 = roicrop(src2, startX, startY, IPC_settings2.getcols(), IPC_settings2.getcols());

      // calculate shifts
      cv::Point2d shifts1 = phasecorrel(crop1, crop2, IPC_settings1);
      cv::Point2d shifts2 = phasecorrel(crop1, crop2, IPC_settings2);

// list results to .csv
#pragma omp critical
      {
        listing << shiftX << "," << shifts1.x << "," << shifts2.x << std::endl;
        progress++;
        std::cout << "> IPC benchmark progress " << progress << " / " << trialsPerStartPos * startFractionX.size() << std::endl;
      }
    }
  }
}

std::tuple<cv::Mat, cv::Mat> calculateFlowMap(const cv::Mat& img1In, const cv::Mat& img2In, IPCsettings& IPC_settings, f64 qualityRatio)
{
  cv::Mat img1 = img1In.clone();
  cv::Mat img2 = img2In.clone();

  i32 rows = qualityRatio * img1.rows;
  i32 cols = qualityRatio * img1.cols;
  i32 win = IPC_settings.getcols();

  cv::Mat flowX = cv::Mat::zeros(rows, cols, CV_32F);
  cv::Mat flowY = cv::Mat::zeros(rows, cols, CV_32F);

  i32 pad = win / 2 + 1;
  volatile i32 progress = 0;

#pragma omp parallel for
  for (i32 r = 0; r < rows; r++)
  {
    for (i32 c = 0; c < cols; c++)
    {
      i32 r_ = (f64)r / qualityRatio;
      i32 c_ = (f64)c / qualityRatio;

      if (c_ < pad or r_ < pad or c_ > (img1.cols - pad) or r_ > (img1.rows - pad))
      {
        flowX.at<f32>(r, c) = 0;
        flowY.at<f32>(r, c) = 0;
      }
      else
      {
        cv::Mat crop1 = roicrop(img1, c_, r_, win, win);
        cv::Mat crop2 = roicrop(img2, c_, r_, win, win);
        auto shift = phasecorrel(crop1, crop2, IPC_settings);
        flowX.at<f32>(r, c) = shift.x;
        flowY.at<f32>(r, c) = shift.y;
      }
    }

#pragma omp critical
    {
      progress++;
      std::cout << "> calculating flow map, " << (f64)progress / rows * 100 << "% done." << std::endl;
    }
  }
  return std::make_tuple(flowX, flowY);
}
