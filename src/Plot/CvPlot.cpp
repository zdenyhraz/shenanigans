#include "CvPlot.hpp"

void CvPlot::RenderInternal()
{
  PROFILE_FUNCTION;
  std::scoped_lock lock(mPlotsMutex);

  while (not mPlots1D.empty())
  {
    RenderInternal(mPlots1D.back());
    mPlots1D.pop_back();
  }

  while (not mPlots2D.empty())
  {
    RenderInternal(mPlots2D.back());
    mPlots2D.pop_back();
  }
}

void CvPlot::RenderInternal(const PlotData1D& data)
{
  PROFILE_FUNCTION;
  LOG_ERROR("CvPlot does not support 1D plots");
}

void CvPlot::RenderInternal(const PlotData2D& data)
{
  PROFILE_FUNCTION;
  cv::Mat image = data.z.clone();
  int height = 600;
  int width = data.aspectratio * height;
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
    cv::normalize(image, image, 0, 255, cv::NORM_MINMAX);
    LOG_DEBUG("Saving image to {}", std::filesystem::weakly_canonical(data.savepath).string());
    cv::imwrite(data.savepath, image);
  }
  else
  {
    cv::imshow(data.name, image);
    cv::waitKey(1);
  }
}
