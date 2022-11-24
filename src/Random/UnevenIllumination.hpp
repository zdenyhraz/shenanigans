#pragma once

void CorrectUnevenIllumination(const cv::Mat& image, i32 tileGridSize, i32 clipLimit)
{
  cv::Mat imageLAB;
  cv::cvtColor(image, imageLAB, cv::COLOR_BGR2Lab);

  std::vector<cv::Mat> LABplanes(3);
  cv::split(imageLAB, LABplanes);

  cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
  clahe->setTilesGridSize({tileGridSize, tileGridSize});
  clahe->setClipLimit(clipLimit);
  clahe->apply(LABplanes[0], LABplanes[0]);

  cv::merge(LABplanes, imageLAB);
  cv::Mat imageCLAHE;
  cv::cvtColor(imageLAB, imageCLAHE, cv::COLOR_Lab2BGR);
  cv::rectangle(imageCLAHE, cv::Rect(0, 0, tileGridSize, tileGridSize), cv::Scalar(0, 0, 255), imageCLAHE.rows * 0.005);

  Saveimg("../data/debug/UnevenIllumination/input_clahe.png", image);
  Saveimg("../data/debug/UnevenIllumination/output_clahe.png", imageCLAHE);
}
