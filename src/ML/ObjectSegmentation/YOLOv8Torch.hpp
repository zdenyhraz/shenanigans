#pragma once
#include <ML/ObjectDetection/YOLOv8Torch.hpp>
#include <ML/ObjectDetection/Draw.hpp>

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
  const auto imageScale = cv::Point2f(static_cast<f32>(source.cols) / modelShape.width, static_cast<f32>(source.rows) / modelShape.height);

  LOG_DEBUG("Input tensor size: [{},{},{},{}]", input.sizes()[0], input.sizes()[1], input.sizes()[2], input.sizes()[3]);
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(input);

  auto model = torch::jit::load(modelPath.string());
  auto outputTuple = model.forward(inputs);

  torch::Tensor output = outputTuple.toTuple()->elements()[0].toTensor(); //(batch_size, num_boxes, num_classes + 4 + num_masks)
  torch::Tensor masks = outputTuple.toTuple()->elements()[1].toTensor();
  LOG_DEBUG("Output tensor1 size: [{},{},{},{}]", output.sizes()[0], output.sizes()[1], output.sizes()[2], output.sizes()[3]);
  LOG_DEBUG("Output tensor2 size: [{},{},{},{}]", masks.sizes()[0], masks.sizes()[1], masks.sizes()[2], masks.sizes()[3]);
  output = output[0].transpose(0, 1); // ignore batch dim + transpose object & box+scores dim

  std::vector<cv::Rect> boxes;
  std::vector<f32> confidences;
  std::vector<i32> classids;
  const i64 objectCount = output.sizes()[0];
  for (i64 i = 0; i < objectCount; ++i)
  {
    //! TODO: this is wrong, output needs to be parsed like in utils.ops.non_max_suppression for masks
    const usize classid = 0;  // TODO
    const f32 confidence = 0; // TODO
    if (confidence < confidenceThreshold)
      continue;

    const auto x = 0; // TODO
    const auto y = 0; // TODO
    const auto w = 0; // TODO
    const auto h = 0; // TODO

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
    objects.emplace_back(boxes[idx], confidences[idx], classids[idx], "xdd");

  Plot::Plot("YOLOv8Torch objects", DrawDetections(source, objects));
  for (i32 i = 0; i < masks.sizes()[1]; ++i)
  {
    auto segmask = ToCVMat(masks[0][i], cv::Size(masks.sizes()[2], masks.sizes()[3]));
    cv::resize(segmask, segmask, source.size());
    Plot::Plot(fmt::format("seg {}", i), segmask);
  }
}
