#pragma once
#include "Draw.hpp"

void SegmentObjectsYOLOv8Torch(const cv::Mat& source, const std::filesystem::path& modelPath, const std::filesystem::path& classesPath, f32 confidenceThreshold, f32 NMSThreshold)
{
  LOG_FUNCTION;
  cv::Size modelShape{640, 640};
  std::vector<std::string> classes;
  std::ifstream inputFile(classesPath);
  if (inputFile.is_open())
  {
    std::string classLine;
    while (std::getline(inputFile, classLine))
      classes.push_back(classLine);
    inputFile.close();
  }

  cv::Mat image = source.clone();
  cv::Mat blob;
  cv::dnn::blobFromImage(image, blob, 1.0 / 255.0, modelShape, cv::Scalar(), true);
  torch::Tensor input = torch::from_blob(blob.data, {1, 3, modelShape.width, modelShape.height});
  f32 scaleX = static_cast<f32>(image.cols) / modelShape.width;
  f32 scaleY = static_cast<f32>(image.rows) / modelShape.height;

  LOG_DEBUG("Input tensor size: [{},{},{},{}]", input.sizes()[0], input.sizes()[1], input.sizes()[2], input.sizes()[3]);
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(input);

  torch::jit::script::Module module;
  {
    LOG_SCOPE("Load network");
    module = torch::jit::load(modelPath.string());
  }
  torch::jit::IValue output;
  {
    LOG_SCOPE("Forward");
    output = module.forward(inputs);
  }

  torch::Tensor t1 = output.toTuple()->elements()[0].toTensor();
  torch::Tensor t2 = output.toTuple()->elements()[1].toTensor();

  LOG_DEBUG("Output tensor1 size: [{},{},{},{}]", t1.sizes()[0], t1.sizes()[1], t1.sizes()[2], t1.sizes()[3]);
  LOG_DEBUG("Output tensor2 size: [{},{},{},{}]", t2.sizes()[0], t2.sizes()[1], t2.sizes()[2], t2.sizes()[3]);
  for (i32 i = 0; i < t2.sizes()[1]; ++i)
    Plot::Plot(fmt::format("seg {}", i), ToCVMat(t2[0][i], cv::Size(t2.sizes()[2], t2.sizes()[3])));
}
}
