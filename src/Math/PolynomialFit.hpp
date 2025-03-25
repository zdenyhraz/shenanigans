#pragma once

inline std::vector<double> PolynomialFit(const std::vector<double>& xdata, const std::vector<double>& ydata, int degree)
{
  int datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, degree + 1, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F);          // matice prave strany
  for (int r = 0; r < X.rows; r++)
  {
    Y.at<float>(r, 0) = ydata[r];
    for (int c = 0; c < X.cols; c++)
    {
      X.at<float>(r, c) = pow(xdata[r], c);
    }
  }
  cv::Mat coeffs = (X.t() * X).inv() * X.t() * Y; // least squares
  cv::Mat fitM = X * coeffs;
  std::vector<double> fit(datacnt, 0);
  for (int r = 0; r < fitM.rows; r++)
  {
    fit[r] = fitM.at<float>(r, 0);
  }
  return fit;
}

inline std::vector<double> PolynomialFit(const std::vector<double>& ydata, int degree)
{
  std::vector<double> xdata(ydata.size());
  std::iota(xdata.begin(), xdata.end(), 1);
  return PolynomialFit(xdata, ydata, degree);
}

inline cv::Mat PolynomialFit(const std::vector<double>& xdata, const std::vector<double>& ydata, const std::vector<double>& zdata, int degree, double xmin, double xmax,
    double ymin, double ymax, double xcnt, double ycnt)
{
  int datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, 2 * degree + 1, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F);              // matice prave strany
  for (int r = 0; r < X.rows; r++)
  {
    Y.at<float>(r, 0) = zdata[r];
    for (int c = 0; c < X.cols; c++)
    {
      if (!c)
        X.at<float>(r, c) = 1;
      else
      {
        if (c % 2)
          X.at<float>(r, c) = pow(xdata[r], ceil((float)c / 2));
        else
          X.at<float>(r, c) = pow(ydata[r], ceil((float)c / 2));
      }
    }
  }
  cv::Mat coeffs = (X.t() * X).inv() * X.t() * Y; // least squares

  cv::Mat fit = cv::Mat::zeros(ycnt, xcnt, CV_32F);
  for (int r = 0; r < fit.rows; r++)
  {
    for (int c = 0; c < fit.cols; c++)
    {
      double x = xmin + ((float)c / (fit.cols - 1)) * (xmax - xmin);
      double y = ymin + ((float)r / (fit.rows - 1)) * (ymax - ymin);
      cv::Mat X1 = cv::Mat::zeros(1, 2 * degree + 1, CV_32F); // matice planu pro 1 point (x,y)
      for (int c1 = 0; c1 < X1.cols; c1++)
      {
        if (!c1)
          X1.at<float>(0, c1) = 1;
        else
        {
          if (c1 % 2)
            X1.at<float>(0, c1) = pow(x, ceil((float)c1 / 2));
          else
            X1.at<float>(0, c1) = pow(y, ceil((float)c1 / 2));
        }
      }

      cv::Mat Z = X1 * coeffs;
      fit.at<float>(r, c) = Z.at<float>(0, 0);
    }
  }
  return fit;
}

inline std::vector<double> PolynomialFitCoefficients(const std::vector<double>& xdata, const std::vector<double>& ydata, int degree)
{
  int datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, degree + 1, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F);          // matice prave strany
  for (int r = 0; r < X.rows; r++)
  {
    Y.at<float>(r, 0) = ydata[r];
    for (int c = 0; c < X.cols; c++)
    {
      X.at<float>(r, c) = pow(xdata[r], c);
    }
  }
  cv::Mat coeffsM = (X.t() * X).inv() * X.t() * Y; // least squares
  std::vector<double> coeffs(coeffsM.rows, 0);
  for (int r = 0; r < coeffsM.rows; r++)
  {
    coeffs[r] = coeffsM.at<float>(r, 0);
  }
  return coeffs;
}
