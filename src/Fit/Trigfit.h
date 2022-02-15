#pragma once

inline std::vector<f64> sin2sin4fit(const std::vector<f64>& xdata, const std::vector<f64>& ydata)
{
  i32 datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, 3, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F); // matice prave strany
  for (i32 r = 0; r < X.rows; r++)
  {
    Y.at<f32>(r, 0) = ydata[r];

    X.at<f32>(r, 0) = 1;
    X.at<f32>(r, 1) = pow(sin(xdata[r]), 2);
    X.at<f32>(r, 2) = pow(sin(xdata[r]), 4);
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

inline std::vector<f64> sin2sin4fitCoeffs(const std::vector<f64>& xdata, const std::vector<f64>& ydata)
{
  i32 datacnt = ydata.size();
  cv::Mat X = cv::Mat::zeros(datacnt, 3, CV_32F); // matice planu
  cv::Mat Y = cv::Mat::zeros(datacnt, 1, CV_32F); // matice prave strany
  for (i32 r = 0; r < X.rows; r++)
  {
    Y.at<f32>(r, 0) = ydata[r];

    X.at<f32>(r, 0) = 1;
    X.at<f32>(r, 1) = pow(sin(xdata[r]), 2);
    X.at<f32>(r, 2) = pow(sin(xdata[r]), 4);
  }
  cv::Mat coeffsM = (X.t() * X).inv() * X.t() * Y; // least squares
  std::vector<f64> coeffs(coeffsM.rows, 0);
  for (i32 r = 0; r < coeffsM.rows; r++)
  {
    coeffs[r] = coeffsM.at<f32>(r, 0);
  }
  return coeffs;
}