#pragma once

inline std::vector<f64> polyfitcore1d(const std::vector<f64>& xdata, const std::vector<f64>& ydata, i32 degree)
{
  i32 datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, degree + 1, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F);          // matice prave strany
  for (i32 r = 0; r < X.rows; r++)
  {
    Y.at<f32>(r, 0) = ydata[r];
    for (i32 c = 0; c < X.cols; c++)
    {
      X.at<f32>(r, c) = pow(xdata[r], c);
    }
  }
  cv::Mat coeffs = (X.t() * X).inv() * X.t() * Y; // least squares
  cv::Mat fitM = X * coeffs;
  std::vector<f64> fit(datacnt, 0);
  for (i32 r = 0; r < fitM.rows; r++)
  {
    fit[r] = fitM.at<f32>(r, 0);
  }
  return fit;
}

inline std::vector<f64> polyfit(const std::vector<f64>& ydata, i32 degree)
{
  std::vector<f64> xdata(ydata.size());
  std::iota(xdata.begin(), xdata.end(), 1);
  return polyfitcore1d(xdata, ydata, degree);
}

inline std::vector<f64> polyfit(const std::vector<f64>& xdata, const std::vector<f64>& ydata, i32 degree)
{
  return polyfitcore1d(xdata, ydata, degree);
}

inline cv::Mat polyfitcore2d(const std::vector<f64>& xdata, const std::vector<f64>& ydata, const std::vector<f64>& zdata, i32 degree, f64 xmin, f64 xmax, f64 ymin, f64 ymax, f64 xcnt, f64 ycnt)
{
  i32 datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, 2 * degree + 1, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F);              // matice prave strany
  for (i32 r = 0; r < X.rows; r++)
  {
    Y.at<f32>(r, 0) = zdata[r];
    for (i32 c = 0; c < X.cols; c++)
    {
      if (!c)
        X.at<f32>(r, c) = 1;
      else
      {
        if (c % 2)
          X.at<f32>(r, c) = pow(xdata[r], ceil((f32)c / 2));
        else
          X.at<f32>(r, c) = pow(ydata[r], ceil((f32)c / 2));
      }
    }
  }
  cv::Mat coeffs = (X.t() * X).inv() * X.t() * Y; // least squares

  cv::Mat fit = cv::Mat::zeros(ycnt, xcnt, CV_32F);
  for (i32 r = 0; r < fit.rows; r++)
  {
    for (i32 c = 0; c < fit.cols; c++)
    {
      f64 x = xmin + ((f32)c / (fit.cols - 1)) * (xmax - xmin);
      f64 y = ymin + ((f32)r / (fit.rows - 1)) * (ymax - ymin);
      cv::Mat X1 = cv::Mat::zeros(1, 2 * degree + 1, CV_32F); // matice planu pro 1 point (x,y)
      for (i32 c1 = 0; c1 < X1.cols; c1++)
      {
        if (!c1)
          X1.at<f32>(0, c1) = 1;
        else
        {
          if (c1 % 2)
            X1.at<f32>(0, c1) = pow(x, ceil((f32)c1 / 2));
          else
            X1.at<f32>(0, c1) = pow(y, ceil((f32)c1 / 2));
        }
      }

      cv::Mat Z = X1 * coeffs;
      fit.at<f32>(r, c) = Z.at<f32>(0, 0);
    }
  }
  return fit;
}

inline cv::Mat polyfit(const std::vector<f64>& xdata, const std::vector<f64>& ydata, const std::vector<f64>& zdata, i32 degree, f64 xmin, f64 xmax, f64 ymin, f64 ymax, f64 xcnt, f64 ycnt)
{
  return polyfitcore2d(xdata, ydata, zdata, degree, xmin, xmax, ymin, ymax, xcnt, ycnt);
}

inline std::vector<f64> polyfitCoeffs(const std::vector<f64>& xdata, const std::vector<f64>& ydata, i32 degree)
{
  i32 datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, degree + 1, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F);          // matice prave strany
  for (i32 r = 0; r < X.rows; r++)
  {
    Y.at<f32>(r, 0) = ydata[r];
    for (i32 c = 0; c < X.cols; c++)
    {
      X.at<f32>(r, c) = pow(xdata[r], c);
    }
  }
  cv::Mat coeffsM = (X.t() * X).inv() * X.t() * Y; // least squares
  std::vector<f64> coeffs(coeffsM.rows, 0);
  for (i32 r = 0; r < coeffsM.rows; r++)
  {
    coeffs[r] = coeffsM.at<f32>(r, 0);
  }
  return coeffs;
}