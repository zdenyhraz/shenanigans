#pragma once

class OnnxModel
{
  Ort::Env env{nullptr};
  Ort::SessionOptions options{nullptr};
  Ort::Session session{nullptr};
  Ort::MemoryInfo memoryInfo{nullptr};
  const char* name = "model";
  std::string logName;
  std::vector<const char*> inputNames;
  std::vector<const char*> outputNames;
  static constexpr bool useCUDA = false;
  static constexpr bool useTensorRT = false;
  bool usesGPU = false;
  bool loaded = false;
  cv::Mat imageTensor;
  Ort::Value inputTensor{nullptr};

  void LoadProviders();
  void Preprocess(const cv::Mat& image);
  static void OnnxLogFunction(void* param, OrtLoggingLevel severity, const char* category, const char* logid, const char* code_location, const char* message);

public:
  OnnxModel(const std::filesystem::path& modelPath, const char* name, const std::vector<const char*>& inputNames, const std::vector<const char*>& outputNames);
  operator bool() const { return loaded; }

  std::vector<Ort::Value> Run(const cv::Mat& image);
  void Load(const std::filesystem::path& modelPath);
  void Unload();
  void SetName(const char* _name) { name = _name; }
  void SetInputNames(const std::vector<const char*>& names) { inputNames = names; }
  void SetOutputNames(const std::vector<const char*>& names) { outputNames = names; }
  void VerifyInputsOutputs();
};
