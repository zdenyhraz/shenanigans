#pragma once
#include "Draw.hpp"

struct Detection
{
  i32 class_id{0};
  std::string className{};
  f32 confidence{0.0};
  cv::Scalar color{};
  cv::Rect box{};
};

void DetectObjectsYOLOv8CV(const cv::Mat& source, const std::filesystem::path& modelPath, const std::filesystem::path& classesPath, f32 confidenceThreshold, f32 NMSThreshold)
{
  cv::Size modelShape{640, 640};

  auto net = cv::dnn::readNetFromONNX(modelPath.string());
  net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

  std::vector<std::string> classes;
  std::ifstream inputFile(classesPath);
  if (inputFile.is_open())
  {
    std::string classLine;
    while (std::getline(inputFile, classLine))
      classes.push_back(classLine);
    inputFile.close();
  }

  cv::Mat input = source.clone();
  cv::Mat blob;
  cv::dnn::blobFromImage(input, blob, 1.0 / 255.0, modelShape, cv::Scalar(), true);
  net.setInput(blob);

  std::vector<cv::Mat> outputs;
  net.forward(outputs, net.getUnconnectedOutLayersNames());
  cv::Mat output = outputs[0];
  LOG_DEBUG("Raw output size: [{},{},{}]", output.size[0], output.size[1], output.size[2]);

  i32 objectCount = output.size[1];
  i32 dimensions = output.size[2];
  bool yolov8 = false;
  // yolov5 has an output of shape (batchSize, 25200, 85) (Num classes + box[x,y,w,h] + confidence[c])
  // yolov8 has an output of shape (batchSize, 84,  8400) (Num classes + box[x,y,w,h])
  if (output.size[2] > output.size[1]) // Check if the shape[2] is more than shape[1] (yolov8)
  {
    yolov8 = true;
    objectCount = output.size[2];
    dimensions = output.size[1];
    output = output.reshape(1, dimensions);
    cv::transpose(output, output);
    LOG_DEBUG("Processed output size: [{},{}]", output.size[0], output.size[1]);
    LOG_DEBUG("Processed output rc: [{},{}]", output.rows, output.cols);
  }
  f32* data = reinterpret_cast<f32*>(output.data);

  f32 x_factor = static_cast<f32>(input.cols) / modelShape.width;
  f32 y_factor = static_cast<f32>(input.rows) / modelShape.height;

  std::vector<i32> class_ids;
  std::vector<f32> confidences;
  std::vector<cv::Rect> boxes;

  for (i32 i = 0; i < objectCount; ++i)
  {
    if (yolov8)
    {
      f32* classes_scores = data + 4;

      cv::Mat scores(1, classes.size(), CV_32FC1, classes_scores);
      cv::Point class_id;
      double maxClassScore;
      cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

      if (maxClassScore > confidenceThreshold)
      {
        f32 x = data[0];
        f32 y = data[1];
        f32 w = data[2];
        f32 h = data[3];

        i32 left = (x - 0.5 * w) * x_factor;
        i32 top = (y - 0.5 * h) * y_factor;
        i32 width = w * x_factor;
        i32 height = h * y_factor;

        class_ids.push_back(class_id.x);
        confidences.push_back(maxClassScore);
        boxes.push_back(cv::Rect(left, top, width, height));
      }
    }
    else // yolov5
    {
      f32 confidence = data[4];

      if (confidence >= confidenceThreshold)
      {
        f32* classes_scores = data + 5;

        cv::Mat scores(1, classes.size(), CV_32FC1, classes_scores);
        cv::Point class_id;
        double max_class_score;

        cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

        if (max_class_score > confidenceThreshold)
        {
          confidences.push_back(confidence);
          class_ids.push_back(class_id.x);

          f32 x = data[0];
          f32 y = data[1];
          f32 w = data[2];
          f32 h = data[3];

          i32 left = (x - 0.5 * w) * x_factor;
          i32 top = (y - 0.5 * h) * y_factor;
          i32 width = w * x_factor;
          i32 height = h * y_factor;

          boxes.push_back(cv::Rect(left, top, width, height));
        }
      }
    }
    data += dimensions;
  }

  std::vector<i32> nms_result;
  cv::dnn::NMSBoxes(boxes, confidences, confidenceThreshold, NMSThreshold, nms_result);
  std::vector<Detection> detections;
  for (usize i = 0; i < nms_result.size(); ++i)
  {
    i32 idx = nms_result[i];
    Detection result;
    result.class_id = class_ids[idx];
    result.confidence = confidences[idx];
    result.className = classes[result.class_id];
    result.box = boxes[idx];
    detections.push_back(result);
  }

  for (const auto& detection : detections)
    DrawPrediction(input, detection.box, detection.className, detection.confidence);

  Plot::Plot("YOLOv8CV objects", input);
}
