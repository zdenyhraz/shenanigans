#pragma once

class OnnxModel
{
  Ort::Env env{nullptr};
  Ort::Session model{nullptr};
  Ort::MemoryInfo memoryInfo{nullptr};
  Ort::SessionOptions options{nullptr};
  const std::vector<const char*> inputNames = {"input"};
  const std::vector<const char*> outputNames = {"boxes", "labels", "scores"};
  bool loaded = false;

  static void LoadProviders(Ort::SessionOptions& options);
  static cv::Mat Preprocess(const cv::Mat& image);

public:
  OnnxModel() = default;
  OnnxModel(const std::filesystem::path& modelPath);
  operator bool() const { return loaded; }

  std::vector<Ort::Value> Run(const cv::Mat image);
  void Load(const std::filesystem::path& modelPath);
  void Unload();
};
