#pragma once

class OnnxModel
{
  Ort::Env env{nullptr};
  Ort::SessionOptions options{nullptr};
  Ort::Session session{nullptr};
  Ort::MemoryInfo memoryInfo{nullptr};
  const char* name = "model";
  std::vector<const char*> inputNames = {"input"};
  std::vector<const char*> outputNames = {"boxes", "labels", "scores"};
  static constexpr bool useCUDA = true;
  static constexpr bool useTensorRT = false;
  bool usesGPU = false;
  bool loaded = false;

  void LoadProviders();
  static cv::Mat Preprocess(const cv::Mat& image);

public:
  OnnxModel(
      const std::filesystem::path& modelPath = "", const char* name = "model", const std::vector<const char*>& inputNames = {}, const std::vector<const char*>& outputNames = {});
  operator bool() const { return loaded; }

  std::vector<Ort::Value> Run(const cv::Mat& image);
  void Load(const std::filesystem::path& modelPath);
  void Unload();
  void SetName(const char* _name) { name = _name; }
  void SetInputNames(const std::vector<const char*>& names) { inputNames = names; }
  void SetOutputNames(const std::vector<const char*>& names) { outputNames = names; }
};
