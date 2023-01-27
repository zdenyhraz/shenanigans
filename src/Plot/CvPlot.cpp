#include "CvPlot.hpp"

void CvPlot::RenderInternal(const PlotData1D& data)
{
  PROFILE_FUNCTION;
  throw std::runtime_error("CvPlot does not support 1D plots");
}

void CvPlot::RenderInternal(const PlotData2D& data)
{
  PROFILE_FUNCTION;
  cv::Mat image = data.z.clone();
  i32 height = 600;
  i32 width = data.aspectratio * height;
  cv::namedWindow(data.name, cv::WINDOW_NORMAL);
  cv::resizeWindow(data.name, width, height);

  if (image.channels() == 1 and data.cmap != "gray")
  {
    cv::Mat cmap;
    cv::normalize(image, image, 0, 255, cv::NORM_MINMAX);
    image.convertTo(image, CV_8U);
    cv::applyColorMap(image, cmap, GetColormap(data.cmap));
    image = cmap;
  }
  else
  {
    cv::normalize(image, image, 0, 1, cv::NORM_MINMAX);
  }

  if (not data.savepath.empty())
  {
  }
  else
  {
    cv::imshow(data.name, image);
    cv::waitKey(1);
  }
}

cv::ColormapTypes CvPlot::GetColormap(const std::string& cmap)
{
  if (cmap == "jet")
    return cv::COLORMAP_JET;

  return cv::COLORMAP_JET;
}
