#include "IPCFlow.hpp"
#include "IPC.hpp"

std::tuple<cv::Mat, cv::Mat> IPCFlow::CalculateFlow(const IPC& ipc, const cv::Mat& image1, const cv::Mat& image2, f64 resolution)
{
  return CalculateFlow(ipc, image1.clone(), image2.clone(), resolution);
}

std::tuple<cv::Mat, cv::Mat> IPCFlow::CalculateFlow(const IPC& ipc, cv::Mat&& image1, cv::Mat&& image2, f64 resolution)
try
{
  PROFILE_FUNCTION;
  if (image1.size() != image2.size())
    throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

  if (ipc.mRows > image1.rows or ipc.mCols > image1.cols)
    throw std::runtime_error(fmt::format("Images are too small ({} < {})", image1.size(), cv::Size(ipc.mCols, ipc.mRows)));

  cv::Mat flowX = cv::Mat(cv::Size(resolution * image1.cols, resolution * image1.rows), GetMatType<IPC::Float>());
  cv::Mat flowY = cv::Mat(cv::Size(resolution * image2.cols, resolution * image2.rows), GetMatType<IPC::Float>());
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 r = 0; r < flowX.rows; ++r)
  {
    if (++progress % (flowX.rows / 20) == 0)
      LOG_DEBUG("Calculating IPC flow profile ({:.0f}%)", static_cast<f64>(progress) / flowX.rows * 100);

    for (i32 c = 0; c < flowX.cols; ++c)
    {
      const cv::Point2i center(c / resolution, r / resolution);

      if (IPC::IsOutOfBounds(center, image1, {ipc.mCols, ipc.mRows}))
        continue;

      const auto shift = ipc.Calculate(RoiCrop(image1, center.x, center.y, ipc.mCols, ipc.mRows), RoiCrop(image2, center.x, center.y, ipc.mCols, ipc.mRows));
      flowX.at<IPC::Float>(r, c) = shift.x;
      flowY.at<IPC::Float>(r, c) = shift.y;
    }
  }

  return {flowX, flowY};
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
  return {};
}
