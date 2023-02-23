#pragma once
#include "Draw.hpp"

struct ObjectDetection
{
  cv::Rect box;
  f32 confidence;
  i32 classid;
  std::string className;
};

std::vector<std::string> LoadClassNames(const std::filesystem::path& classesPath)
{
  LOG_FUNCTION;
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

std::vector<torch::jit::IValue> Preprocess(const cv::Mat& source, cv::Size modelShape)
{
  LOG_FUNCTION;
  static cv::Mat blob;
  cv::dnn::blobFromImage(source, blob, 1.0 / 255.0, modelShape, cv::Scalar(), true);
  torch::Tensor input = torch::from_blob(blob.data, {1, 3, modelShape.width, modelShape.height});
  LOG_DEBUG("Input tensor size: [{},{},{},{}]", input.sizes()[0], input.sizes()[1], input.sizes()[2], input.sizes()[3]);
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(input);
  return inputs;
}

torch::jit::script::Module LoadModel(const std::filesystem::path& modelPath)
{
  LOG_FUNCTION;
  return torch::jit::load(modelPath.string());
}

torch::Tensor Forward(torch::jit::script::Module& model, std::vector<torch::jit::IValue>& inputs)
{
  LOG_FUNCTION;
  return model.forward(inputs).toTensor();
}

cv::Mat DrawDetections(const cv::Mat source, const std::vector<ObjectDetection>& objects)
{
  LOG_FUNCTION;
  auto image = source.clone();
  for (const auto& object : objects)
    DrawPrediction(image, object.box, object.className, object.confidence);
  return image;
}

std::vector<ObjectDetection> Postprocess(
    torch::Tensor& output, f32 confidenceThreshold, f32 NMSThreshold, const std::vector<std::string>& classes, cv::Point2f imageScale, const cv::Mat source)
{
  LOG_FUNCTION;
  LOG_DEBUG("Raw output tensor size: [{},{},{},{}]", output.sizes()[0], output.sizes()[1], output.sizes()[2], output.sizes()[3]);
  output = output[0].transpose(0, 1); // ignore batch dim + transpose object & box+scores dim
  LOG_DEBUG("Processed output tensor size: [{},{},{},{}]", output.sizes()[0], output.sizes()[1], output.sizes()[2], output.sizes()[3]);
  std::vector<cv::Rect> boxes;
  std::vector<f32> confidences;
  std::vector<i32> classids;
  const i64 objectCount = output.sizes()[0];
  for (i64 i = 0; i < objectCount; ++i)
  {
    const usize classid = torch::argmax(output[i].slice(0, 4)).item<f32>(); // box[x,y,w,h] + class scores
    const f32 confidence = output[i].slice(0, 4)[classid].item<f32>();

    if (confidence < confidenceThreshold)
      continue;

    const auto x = output[i][0].item<f32>();
    const auto y = output[i][1].item<f32>();
    const auto w = output[i][2].item<f32>();
    const auto h = output[i][3].item<f32>();

    i32 left = (x - 0.5 * w) * imageScale.x;
    i32 top = (y - 0.5 * h) * imageScale.y;
    i32 width = w * imageScale.x;
    i32 height = h * imageScale.y;

    boxes.push_back(cv::Rect(left, top, width, height));
    confidences.push_back(confidence);
    classids.push_back(classid);
  }

  std::vector<i32> boxesIdx;
  cv::dnn::NMSBoxes(boxes, confidences, confidenceThreshold, NMSThreshold, boxesIdx);

  std::vector<ObjectDetection> objects;
  for (const auto idx : boxesIdx)
    objects.emplace_back(boxes[idx], confidences[idx], classids[idx], classes[classids[idx]]);
  return objects;
}

void DetectObjectsYOLOv8Torch(const cv::Mat& source, const std::filesystem::path& modelPath, const std::filesystem::path& classesPath, f32 confidenceThreshold, f32 NMSThreshold)
{
  LOG_FUNCTION;
  const auto modelShape = cv::Size(640, 640);
  const auto imageScale = cv::Point2f(static_cast<f32>(source.cols) / modelShape.width, static_cast<f32>(source.rows) / modelShape.height);
  const auto classes = LoadClassNames(classesPath);
  auto model = LoadModel(modelPath);
  auto inputs = Preprocess(source, modelShape);
  auto output = Forward(model, inputs);
  const auto objects = Postprocess(output, confidenceThreshold, NMSThreshold, classes, imageScale, source);
  Plot::Plot("YOLOv8Torch objects", DrawDetections(source, objects));
}
