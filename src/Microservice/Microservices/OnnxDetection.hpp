#pragma once
#include "Microservice/Microservice.hpp"
#include "ONNX/OnnxModel.hpp"

class OnnxDetection : public Microservice
{
  std::optional<OnnxModel> model;
  std::vector<std::string> classNames;
  const float scoreThreshold = 0.5;

  struct Detection
  {
    cv::Rect box;
    float score = 0;
    int64_t label = 0;
  };

  void Load() override { model = OnnxModel(GetProjectPath(GetParameter<std::string>("model path")), "onnx_detection_perf", {"input"}, {"boxes", "labels", "scores"}); }

  void Unload() override { model->Unload(); }

  void Process() override
  {
    LOG_SCOPE("OnnxDetection");
    const auto outputTensor = model->Run(GetInputParameter<cv::Mat>("image"));
    const auto detections = ParseModelOutput(outputTensor);
    LOG_DEBUG("Detected {} objects", detections.size());
    std::vector<cv::Rect> boxes;
    boxes.reserve(detections.size());
    for (const auto& det : detections)
      boxes.push_back(det.box);
    SetOutputParameter("objects", boxes);
  }

  std::vector<Detection> ParseModelOutput(const std::vector<Ort::Value>& outputTensor)
  {
    if (outputTensor.size() != 3)
      throw EXCEPTION("Model output size mismatch (expected 3, got {})", outputTensor.size());
    size_t detectionCount = outputTensor[0].GetTensorTypeAndShapeInfo().GetShape()[0];
    const float* boxes = outputTensor[0].GetTensorData<float>();
    const int64_t* labels = outputTensor[1].GetTensorData<int64_t>();
    const float* scores = outputTensor[2].GetTensorData<float>();

    std::vector<Detection> detections;
    detections.reserve(detectionCount);
    for (size_t idx = 0; idx < detectionCount; ++idx)
      if (scores[idx] > scoreThreshold)
      {
        cv::Rect box(boxes[idx * 4 + 0], boxes[idx * 4 + 1], boxes[idx * 4 + 2] - boxes[idx * 4], boxes[idx * 4 + 3] - boxes[idx * 4 + 1]);
        detections.emplace_back(box, scores[idx], labels[idx]);
      }

    return detections;
  }

public:
  OnnxDetection()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineOutputParameter<std::vector<cv::Rect>>("objects");
    DefineParameter<std::string>("model path", "");
  }
};
