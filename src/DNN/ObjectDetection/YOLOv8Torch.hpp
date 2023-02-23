#pragma once
#include "Draw.hpp"

std::vector<std::string> LoadClassNames(const std::filesystem::path& classesPath)
{
  std::vector<std::string> classes;
  std::ifstream inputFile(classesPath);
  if (inputFile.is_open())
  {
    std::string classLine;
    while (std::getline(inputFile, classLine))
      classes.push_back(classLine);
    inputFile.close();
  }
  return classes;
}

void DetectObjectsYOLOv8Torch(const cv::Mat& source, const std::filesystem::path& modelPath, const std::filesystem::path& classesPath, f32 confidenceThreshold, f32 NMSThreshold)
{
  LOG_FUNCTION;
  const auto modelShape = cv::Size(640, 640);
  const auto classes = LoadClassNames(classesPath);

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

  torch::Tensor outputTensor = output.toTensor();
  LOG_DEBUG("Raw output tensor size: [{},{},{},{}]", outputTensor.sizes()[0], outputTensor.sizes()[1], outputTensor.sizes()[2], outputTensor.sizes()[3]);
  outputTensor = outputTensor[0].transpose(0, 1); // ignore batch dim + transpose object & box+scores dim
  LOG_DEBUG("Processed output tensor size: [{},{},{},{}]", outputTensor.sizes()[0], outputTensor.sizes()[1], outputTensor.sizes()[2], outputTensor.sizes()[3]);
  const i64 objectCount = outputTensor.sizes()[0];
  std::vector<i32> classids;
  std::vector<f32> confidences;
  std::vector<cv::Rect> boxes;
  for (i64 i = 0; i < objectCount; ++i)
  {
    const usize classid = torch::argmax(outputTensor[i].slice(0, 4)).item<f32>(); // box[x,y,w,h] + class scores
    const f32 confidence = outputTensor[i].slice(0, 4)[classid].item<f32>();

    if (confidence < confidenceThreshold)
      continue;

    const auto x = outputTensor[i][0].item<f32>();
    const auto y = outputTensor[i][1].item<f32>();
    const auto w = outputTensor[i][2].item<f32>();
    const auto h = outputTensor[i][3].item<f32>();
    const std::string className = classes[classid];

    i32 left = (x - 0.5 * w) * scaleX;
    i32 top = (y - 0.5 * h) * scaleY;
    i32 width = w * scaleX;
    i32 height = h * scaleY;

    classids.push_back(classid);
    confidences.push_back(confidence);
    boxes.push_back(cv::Rect(left, top, width, height));
  }

  std::vector<i32> boxesIdx;
  cv::dnn::NMSBoxes(boxes, confidences, confidenceThreshold, NMSThreshold, boxesIdx);

  for (const auto idx : boxesIdx)
    DrawPrediction(image, boxes[idx], classes[classids[idx]], confidences[idx]);

  Plot::Plot("YOLOv8Torch objects", image);
}
