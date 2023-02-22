#pragma once

inline void Preprocess(cv::Mat& frame, cv::dnn::Net& net, cv::Size inpSize, float scale, const cv::Scalar& mean, bool swapRB)
{
  static cv::Mat blob;
  if (inpSize.width <= 0)
    inpSize.width = frame.cols;
  if (inpSize.height <= 0)
    inpSize.height = frame.rows;

  if (frame.channels() == 1)
  {
    cv::normalize(frame, frame, 0, 255, cv::NORM_MINMAX);
    frame.convertTo(frame, CV_8U);
    cv::Mat cmap;
    cv::applyColorMap(frame, cmap, cv::COLORMAP_VIRIDIS);
    frame = cmap;
  }

  cv::dnn::blobFromImage(frame, blob, 1.0, inpSize, cv::Scalar(), swapRB, false, CV_8U);

  net.setInput(blob, "", scale, mean);
}

void DrawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame, const std::vector<std::string>& classes)
{
  const auto boxThickness = std::clamp(0.007 * frame.rows, 1., 100.);
  const auto fontThickness = std::clamp(0.003 * frame.rows, 1., 100.);
  const auto color = cv::Scalar(Random::Rand<i32>(100, 255), Random::Rand<i32>(100, 255), Random::Rand<i32>(100, 255));
  const f64 fontScale = 1.5 * frame.rows / 1600;
  const auto font = cv::FONT_HERSHEY_SIMPLEX;
  const f64 labelPaddingX = 0.2;
  const f64 labelPaddingY = 0.5;

  cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), color, boxThickness);

  std::string label = fmt::format("{:.2f}", conf);
  if (not classes.empty())
    label = classes[classId] + ": " + label;

  cv::Size labelSize = cv::getTextSize(label, font, fontScale, fontThickness, nullptr);
  cv::rectangle(
      frame, cv::Point(left - 0.5 * boxThickness, top - labelSize.height * (1. + labelPaddingY)), cv::Point(left + labelSize.width * (1. + labelPaddingX), top), color, cv::FILLED);
  cv::putText(frame, label, cv::Point(left + 0.5 * labelPaddingX * labelSize.width, top - 0.5 * labelPaddingY * labelSize.height), font, fontScale, cv::Scalar::all(0),
      fontThickness, cv::LINE_AA);
}

void Postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs, cv::dnn::Net& net, int backend, float confThreshold, float nmsThreshold, const std::vector<std::string>& classes)
{
  static std::vector<int> outLayers = net.getUnconnectedOutLayers();
  static std::string outLayerType = net.getLayer(outLayers[0])->type;

  std::vector<int> classIds;
  std::vector<float> confidences;
  std::vector<cv::Rect> boxes;
  if (outLayerType == "DetectionOutput")
  {
    // Network produces output blob with a shape 1x1xNx7 where N is a number of
    // detections and an every detection is a vector of values
    // [batchId, classId, confidence, left, top, right, bottom]
    for (size_t k = 0; k < outs.size(); k++)
    {
      float* data = reinterpret_cast<float*>(outs[k].data);
      for (size_t i = 0; i < outs[k].total(); i += 7)
      {
        float confidence = data[i + 2];
        if (confidence > confThreshold)
        {
          int left = data[i + 3];
          int top = data[i + 4];
          int right = data[i + 5];
          int bottom = data[i + 6];
          int width = right - left + 1;
          int height = bottom - top + 1;
          if (width <= 2 || height <= 2)
          {
            left = data[i + 3] * frame.cols;
            top = data[i + 4] * frame.rows;
            right = data[i + 5] * frame.cols;
            bottom = data[i + 6] * frame.rows;
            width = right - left + 1;
            height = bottom - top + 1;
          }
          classIds.push_back(data[i + 1] - 1); // Skip 0th background class id.
          boxes.push_back(cv::Rect(left, top, width, height));
          confidences.push_back(confidence);
        }
      }
    }
  }
  else if (outLayerType == "Region")
  {
    for (size_t i = 0; i < outs.size(); ++i)
    {
      // Network produces output blob with a shape NxC where N is a number of
      // detected objects and C is a number of classes + 4 where the first 4
      // numbers are [center_x, center_y, width, height]
      float* data = reinterpret_cast<float*>(outs[i].data);
      for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
      {
        cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
        cv::Point classIdPoint;
        double confidence;
        cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
        if (confidence > confThreshold)
        {
          int centerX = data[0] * frame.cols;
          int centerY = data[1] * frame.rows;
          int width = data[2] * frame.cols;
          int height = data[3] * frame.rows;
          int left = centerX - width / 2;
          int top = centerY - height / 2;

          classIds.push_back(classIdPoint.x);
          confidences.push_back(confidence);
          boxes.push_back(cv::Rect(left, top, width, height));
        }
      }
    }
  }
  else
    throw std::runtime_error("Unknown output layer type: " + outLayerType);

  // NMS is used inside Region layer only on DNN_BACKEND_OPENCV for another backends we need NMS in sample
  // or NMS is required if number of outputs > 1
  if (outLayers.size() > 1 or (outLayerType == "Region" and backend != cv::dnn::DNN_BACKEND_OPENCV))
  {
    std::map<int, std::vector<size_t>> class2indices;
    for (size_t i = 0; i < classIds.size(); i++)
      if (confidences[i] >= confThreshold)
        class2indices[classIds[i]].push_back(i);

    std::vector<cv::Rect> nmsBoxes;
    std::vector<float> nmsConfidences;
    std::vector<int> nmsClassIds;
    for (std::map<int, std::vector<size_t>>::iterator it = class2indices.begin(); it != class2indices.end(); ++it)
    {
      std::vector<cv::Rect> localBoxes;
      std::vector<float> localConfidences;
      std::vector<size_t> classIndices = it->second;
      for (size_t i = 0; i < classIndices.size(); i++)
      {
        localBoxes.push_back(boxes[classIndices[i]]);
        localConfidences.push_back(confidences[classIndices[i]]);
      }
      std::vector<int> nmsIndices;
      cv::dnn::NMSBoxes(localBoxes, localConfidences, confThreshold, nmsThreshold, nmsIndices);
      for (size_t i = 0; i < nmsIndices.size(); i++)
      {
        size_t idx = nmsIndices[i];
        nmsBoxes.push_back(localBoxes[idx]);
        nmsConfidences.push_back(localConfidences[idx]);
        nmsClassIds.push_back(it->first);
      }
    }
    boxes = nmsBoxes;
    classIds = nmsClassIds;
    confidences = nmsConfidences;
  }

  for (size_t idx = 0; idx < boxes.size(); ++idx)
  {
    cv::Rect box = boxes[idx];
    DrawPred(classIds[idx], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height, frame, classes);
  }
}

void DetectObjectsYOLOv3CV(const cv::Mat& image, const std::filesystem::path& modelPath, const std::filesystem::path& configPath, const std::string& framework,
    const std::filesystem::path& classesPath, float confThreshold)
{
  LOG_FUNCTION;
  float nmsThreshold = 0.2;
  std::vector<std::string> classes;
  float scale = 1. / 255.;
  cv::Scalar mean = cv::Scalar(0);
  bool swapRB = true;
  cv::Size inputSize(416, 416);

  std::ifstream ifs(classesPath.c_str());
  if (!ifs.is_open())
    throw std::invalid_argument("File " + classesPath.string() + " not found");
  std::string line;
  while (std::getline(ifs, line))
    classes.push_back(line);

  cv::dnn::Net net = cv::dnn::readNet(modelPath.string(), configPath.string(), framework);
  const auto backend = cv::dnn::DNN_BACKEND_OPENCV;
  const auto target = cv::dnn::DNN_TARGET_CPU;
  net.setPreferableBackend(backend);
  net.setPreferableTarget(target);
  std::vector<cv::String> outNames = net.getUnconnectedOutLayersNames();

  cv::Mat frame = image.clone();
  Preprocess(frame, net, inputSize, scale, mean, swapRB);
  std::vector<cv::Mat> outs;
  net.forward(outs, outNames);
  Postprocess(frame, outs, net, backend, confThreshold, nmsThreshold, classes);

  Plot::Plot("objects DNN", frame);
}
