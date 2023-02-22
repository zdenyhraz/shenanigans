#pragma once
#include "Draw.hpp"

cv::Mat RGB2BGR(const cv::Mat& rgb)
{
  cv::Mat bgr = rgb.clone();
  cv::cvtColor(bgr, bgr, cv::COLOR_RGB2BGR);
  return bgr;
}

void DetectObjectsYOLOv8Torch(const cv::Mat& image, const std::filesystem::path& networkPath, f32 confidenceThreshold)
{
  LOG_FUNCTION;
  LOG_DEBUG("Loading network {}", networkPath.string());
  if (not std::filesystem::exists(networkPath))
    throw std::invalid_argument(fmt::format("File {} does not exist", networkPath.string()));

  LOG_DEBUG("Input raw image size: [{},{},{}]", image.channels(), image.cols, image.rows);

  cv::Mat inputCV = image.clone();
  cv::resize(inputCV, inputCV, cv::Size(640, 640)); // YOLO 640x640 input
  if (inputCV.channels() == 1)
    cv::cvtColor(inputCV, inputCV, cv::COLOR_GRAY2RGB); // YOLO RGB input
  else if (inputCV.channels() == 3)
    cv::cvtColor(inputCV, inputCV, cv::COLOR_BGR2RGB); // YOLO RGB input
  else
    throw std::invalid_argument("YOLO accepts only RGB images");
  inputCV.convertTo(inputCV, CV_32FC3);
  cv::normalize(inputCV, inputCV, 0, 1, cv::NORM_MINMAX);
  Plot::Plot("input", RGB2BGR(inputCV));

  LOG_DEBUG("Input preprocessed image size: [{},{},{}]", inputCV.channels(), inputCV.cols, inputCV.rows);
  torch::Tensor input = ToTensor(inputCV).reshape({1, inputCV.channels(), inputCV.rows, inputCV.cols});
  LOG_DEBUG("Input tensor size: [{},{},{},{}]", input.sizes()[0], input.sizes()[1], input.sizes()[2], input.sizes()[3]);
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(input);

  torch::jit::script::Module module;
  {
    LOG_SCOPE("Load network");
    module = torch::jit::load(networkPath.string());
  }
  torch::jit::IValue output;
  {
    LOG_SCOPE("Forward");
    output = module.forward(inputs);
  }

  if (networkPath.string().find("seg") != std::string::npos) // segment
  {
    torch::Tensor t1 = output.toTuple()->elements()[0].toTensor();
    torch::Tensor t2 = output.toTuple()->elements()[1].toTensor();

    LOG_DEBUG("Output tensor1 size: [{},{},{},{}]", t1.sizes()[0], t1.sizes()[1], t1.sizes()[2], t1.sizes()[3]);
    LOG_DEBUG("Output tensor2 size: [{},{},{},{}]", t2.sizes()[0], t2.sizes()[1], t2.sizes()[2], t2.sizes()[3]);
    for (i32 i = 0; i < t2.sizes()[1]; ++i)
    {
      Plot::Plot(fmt::format("seg {}", i), ToCVMat(t2[0][i], cv::Size(t2.sizes()[2], t2.sizes()[3])));
    }
  }
  else // detect
  {
    torch::Tensor outputTensor = output.toTensor();
    LOG_DEBUG("Output tensor size: [{},{},{},{}]", outputTensor.sizes()[0], outputTensor.sizes()[1], outputTensor.sizes()[2], outputTensor.sizes()[3]);
    const i32 objectCount = static_cast<i32>(outputTensor.sizes()[2]);
    for (i32 i = 0; i < objectCount; ++i)
    {
      const i32 x = outputTensor[0][0][i].item<f32>();
      const i32 y = outputTensor[0][1][i].item<f32>();
      const i32 w = outputTensor[0][2][i].item<f32>();
      const i32 h = outputTensor[0][3][i].item<f32>();
      const f32 confidence = outputTensor[0][4][i].item<f32>();

      if (confidence < confidenceThreshold)
        continue;

      DrawPrediction(inputCV, cv::Rect(x, y, w, h), "xddd", confidence);
    }
    Plot::Plot("YOLOv8Torch objects", RGB2BGR(inputCV));
  }
}
